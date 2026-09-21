#ifndef RAXTER_WAVE_H
#define RAXTER_WAVE_H

#include "../core/raxter_types.h"
#include "vec.h"
#include <math.h>

#define RAXTER_WAVE_MAX_WAVELENGTHS 32
#define RAXTER_WAVE_MAX_SOURCES 64
#define RAXTER_MAX_SOURCES RAXTER_WAVE_MAX_SOURCES
#define RAXTER_WAVE_MAX_BOUNCES 8

typedef struct {
    f32 wavelength_nm;
    f32 amplitude;
    f32 phase;
    vec3 polarization;
} wave_component;

typedef struct {
    wave_component components[RAXTER_WAVE_MAX_WAVELENGTHS];
    u32 count;
} wave_spectrum;

typedef struct {
    vec3 position;
    vec3 direction;
    wave_spectrum spectrum;
    f32 intensity;
    f32 coherence_length;
    bool is_coherent;
} light_source;

typedef struct {
    vec3 position;
    vec3 normal;
    vec3 k_vector;
    wave_spectrum incident;
    wave_spectrum reflected;
    wave_spectrum transmitted;
    f32 phase_shift;
    u32 bounce_count;
} wave_interaction;

typedef struct {
    f32 refractive_index_real;
    f32 refractive_index_imag;
    f32 absorption_coefficient;
    f32 scattering_coefficient;
    vec3 anisotropy;
} material_optical;

static inline wave_spectrum wave_spectrum_zero() {
    return (wave_spectrum){{{0}}, 0};
}

static inline wave_spectrum wave_spectrum_single(f32 wavelength_nm, f32 amplitude, vec3 polarization) {
    wave_spectrum s = wave_spectrum_zero();
    s.components[0] = (wave_component){
        .wavelength_nm = wavelength_nm,
        .amplitude = amplitude,
        .phase = 0.0f,
        .polarization = vec3_normalize(polarization),
    };
    s.count = 1;
    return s;
}

static inline wave_spectrum wave_spectrum_add(wave_spectrum a, wave_spectrum b) {
    wave_spectrum result = a;
    for (u32 i = 0; i < b.count; i++) {
        bool found = false;
        for (u32 j = 0; j < result.count; j++) {
            if (fabsf(result.components[j].wavelength_nm - b.components[i].wavelength_nm) < 1.0f) {
                result.components[j].amplitude += b.components[i].amplitude;
                result.components[j].phase = atan2f(
                    result.components[j].amplitude * sinf(result.components[j].phase) +
                    b.components[i].amplitude * sinf(b.components[i].phase),
                    result.components[j].amplitude * cosf(result.components[j].phase) +
                    b.components[i].amplitude * cosf(b.components[i].phase)
                );
                found = true;
                break;
            }
        }
        if (!found && result.count < RAXTER_WAVE_MAX_WAVELENGTHS) {
            result.components[result.count++] = b.components[i];
        }
    }
    return result;
}

static inline f32 wave_spectrum_intensity(wave_spectrum s) {
    f32 total = 0.0f;
    for (u32 i = 0; i < s.count; i++) {
        total += s.components[i].amplitude * s.components[i].amplitude;
    }
    return total;
}

static inline vec3 wave_spectrum_color(wave_spectrum s) {
    vec3 color = {0, 0, 0};
    for (u32 i = 0; i < s.count; i++) {
        f32 lambda = s.components[i].wavelength_nm;
        vec3 xyz = {0, 0, 0};
        
        if (lambda >= 380 && lambda < 440) {
            xyz.x = -(lambda - 440) / (440 - 380);
            xyz.y = 0.0f;
            xyz.z = 1.0f;
        } else if (lambda >= 440 && lambda < 490) {
            xyz.x = 0.0f;
            xyz.y = (lambda - 440) / (490 - 440);
            xyz.z = 1.0f;
        } else if (lambda >= 490 && lambda < 510) {
            xyz.x = 0.0f;
            xyz.y = 1.0f;
            xyz.z = -(lambda - 510) / (510 - 490);
        } else if (lambda >= 510 && lambda < 580) {
            xyz.x = (lambda - 510) / (580 - 510);
            xyz.y = 1.0f;
            xyz.z = 0.0f;
        } else if (lambda >= 580 && lambda < 645) {
            xyz.x = 1.0f;
            xyz.y = -(lambda - 645) / (645 - 580);
            xyz.z = 0.0f;
        } else if (lambda >= 645 && lambda <= 780) {
            xyz.x = 1.0f;
            xyz.y = 0.0f;
            xyz.z = 0.0f;
        }
        
        f32 factor = 1.0f;
        if (lambda < 420) factor = 0.3f + 0.7f * (lambda - 380) / (420 - 380);
        else if (lambda > 700) factor = 0.3f + 0.7f * (780 - lambda) / (780 - 700);
        
        f32 amp = s.components[i].amplitude * factor;
        color = vec3_add(color, vec3_scale(xyz, amp * amp));
    }
    return color;
}

static inline f32 fresnel_schlick(f32 cos_theta, f32 f0) {
    return f0 + (1.0f - f0) * powf(1.0f - cos_theta, 5.0f);
}

static inline f32 fresnel_exact(f32 cos_theta_i, f32 eta_i, f32 eta_t) {
    f32 sin_theta_t_sq = (eta_i / eta_t) * (eta_i / eta_t) * (1.0f - cos_theta_i * cos_theta_i);
    if (sin_theta_t_sq >= 1.0f) return 1.0f;
    
    f32 cos_theta_t = sqrtf(1.0f - sin_theta_t_sq);
    
    f32 rs = (eta_i * cos_theta_i - eta_t * cos_theta_t) / (eta_i * cos_theta_i + eta_t * cos_theta_t);
    f32 rp = (eta_t * cos_theta_i - eta_i * cos_theta_t) / (eta_t * cos_theta_i + eta_i * cos_theta_t);
    
    return 0.5f * (rs * rs + rp * rp);
}

static inline vec3 compute_polarization_reflection(vec3 incident_pol, vec3 normal, f32 cos_theta_i, f32 eta_i, f32 eta_t) {
    vec3 s_pol = vec3_normalize(vec3_cross(incident_pol, normal));
    vec3 p_pol = vec3_normalize(vec3_cross(s_pol, incident_pol));
    
    f32 sin_theta_t_sq = (eta_i / eta_t) * (eta_i / eta_t) * (1.0f - cos_theta_i * cos_theta_i);
    if (sin_theta_t_sq >= 1.0f) {
        return incident_pol;
    }
    
    f32 cos_theta_t = sqrtf(1.0f - sin_theta_t_sq);
    
    f32 rs = (eta_i * cos_theta_i - eta_t * cos_theta_t) / (eta_i * cos_theta_i + eta_t * cos_theta_t);
    f32 rp = (eta_t * cos_theta_i - eta_i * cos_theta_t) / (eta_t * cos_theta_i + eta_i * cos_theta_t);
    
    return vec3_add(vec3_scale(s_pol, rs), vec3_scale(p_pol, rp));
}

static inline f32 phase_shift_reflection(f32 cos_theta_i, f32 eta_i, f32 eta_t) {
    f32 sin_theta_t_sq = (eta_i / eta_t) * (eta_i / eta_t) * (1.0f - cos_theta_i * cos_theta_i);
    if (sin_theta_t_sq >= 1.0f) return RAXTER_PI;
    
    f32 cos_theta_t = sqrtf(1.0f - sin_theta_t_sq);
    f32 rp = (eta_t * cos_theta_i - eta_i * cos_theta_t) / (eta_t * cos_theta_i + eta_i * cos_theta_t);
    
    return rp < 0 ? RAXTER_PI : 0.0f;
}

#endif