#ifndef RAXTER_SPECTRUM_H
#define RAXTER_SPECTRUM_H

#include "../core/raxter_types.h"
#include "vec.h"
#include <math.h>

#define RAXTER_SPECTRUM_SAMPLES 32
#define RAXTER_CIE_SAMPLES 55

typedef struct {
    f32 samples[RAXTER_SPECTRUM_SAMPLES];
    f32 lambda_min;
    f32 lambda_max;
} spectral_power;

typedef struct {
    f32 x, y, z;
} cie_xyz;

static const f32 CIE_LAMBDA_MIN = 360.0f;
static const f32 CIE_LAMBDA_MAX = 830.0f;

extern const f32 CIE_X[RAXTER_CIE_SAMPLES];
extern const f32 CIE_Y[RAXTER_CIE_SAMPLES];
extern const f32 CIE_Z[RAXTER_CIE_SAMPLES];

static inline spectral_power spectrum_make_uniform(f32 value, f32 lambda_min, f32 lambda_max) {
    spectral_power s = {0};
    s.lambda_min = lambda_min;
    s.lambda_max = lambda_max;
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        s.samples[i] = value;
    }
    return s;
}

static inline spectral_power spectrum_make_blackbody(f32 temperature_k) {
    spectral_power s = {0};
    s.lambda_min = 380.0f;
    s.lambda_max = 780.0f;
    
    const f32 h = 6.62607015e-34f;
    const f32 c = 299792458.0f;
    const f32 k_b = 1.380649e-23f;
    
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        f32 lambda = s.lambda_min + (s.lambda_max - s.lambda_min) * i / (RAXTER_SPECTRUM_SAMPLES - 1);
        f32 lambda_m = lambda * 1e-9f;
        
        f32 numerator = 2.0f * h * c * c;
        f32 denominator = powf(lambda_m, 5.0f) * (expf(h * c / (lambda_m * k_b * temperature_k)) - 1.0f);
        s.samples[i] = numerator / denominator;
    }
    
    f32 max_val = 0.0f;
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        if (s.samples[i] > max_val) max_val = s.samples[i];
    }
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        s.samples[i] /= max_val;
    }
    
    return s;
}

static inline spectral_power spectrum_add(spectral_power a, spectral_power b) {
    spectral_power result = a;
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        result.samples[i] += b.samples[i];
    }
    return result;
}

static inline spectral_power spectrum_scale(spectral_power a, f32 s) {
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        a.samples[i] *= s;
    }
    return a;
}

static inline spectral_power spectrum_mul(spectral_power a, spectral_power b) {
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        a.samples[i] *= b.samples[i];
    }
    return a;
}

static inline cie_xyz spectrum_to_xyz(spectral_power s) {
    cie_xyz xyz = {0, 0, 0};
    f32 delta_lambda = (s.lambda_max - s.lambda_min) / (RAXTER_SPECTRUM_SAMPLES - 1);
    
    for (int i = 0; i < RAXTER_SPECTRUM_SAMPLES; i++) {
        f32 lambda = s.lambda_min + i * delta_lambda;
        int cie_idx = (int)((lambda - CIE_LAMBDA_MIN) / (CIE_LAMBDA_MAX - CIE_LAMBDA_MIN) * (RAXTER_CIE_SAMPLES - 1));
        cie_idx = RAXTER_CLAMP(cie_idx, 0, RAXTER_CIE_SAMPLES - 1);
        
        xyz.x += s.samples[i] * CIE_X[cie_idx] * delta_lambda;
        xyz.y += s.samples[i] * CIE_Y[cie_idx] * delta_lambda;
        xyz.z += s.samples[i] * CIE_Z[cie_idx] * delta_lambda;
    }
    
    return xyz;
}

static inline f32 srgb_comp_func(f32 c) {
    return c <= 0.0031308f ? 12.92f * c : 1.055f * powf(c, 1.0f/2.4f) - 0.055f;
}

static inline vec3 xyz_to_srgb(cie_xyz xyz) {
    f32 r =  3.2406f * xyz.x - 1.5372f * xyz.y - 0.4986f * xyz.z;
    f32 g = -0.9689f * xyz.x + 1.8758f * xyz.y + 0.0415f * xyz.z;
    f32 b =  0.0557f * xyz.x - 0.2040f * xyz.y + 1.0570f * xyz.z;
    
    return (vec3){
        RAXTER_CLAMP(srgb_comp_func(r), 0.0f, 1.0f),
        RAXTER_CLAMP(srgb_comp_func(g), 0.0f, 1.0f),
        RAXTER_CLAMP(srgb_comp_func(b), 0.0f, 1.0f),
    };
}

static inline vec3 spectrum_to_srgb(spectral_power s) {
    return xyz_to_srgb(spectrum_to_xyz(s));
}

static inline f32 spectrum_luminance(spectral_power s) {
    cie_xyz xyz = spectrum_to_xyz(s);
    return xyz.y;
}

#endif