#include "light_transport.h"
#include "math/vec.h"
#include "math/wave.h"
#include "math/spectrum.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

static u64 pcg32_state = 0x853c49e6748fea9bULL;
static u64 pcg32_inc = 0xda3e39cb94b95bdbULL;

static u32 pcg32_random() {
    u64 oldstate = pcg32_state;
    pcg32_state = oldstate * 6364136223846793005ULL + (pcg32_inc | 1);
    u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    u32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static f32 pcg32_random_float() {
    return (f32)pcg32_random() / (f32)UINT32_MAX;
}

static vec3 random_hemisphere(vec3 normal) {
    vec3 tangent = fabsf(normal.x) > 0.5f ? (vec3){0, 1, 0} : (vec3){1, 0, 0};
    tangent = vec3_normalize(vec3_cross(tangent, normal));
    vec3 bitangent = vec3_cross(normal, tangent);
    
    f32 u1 = pcg32_random_float();
    f32 u2 = pcg32_random_float();
    f32 r = sqrtf(u1);
    f32 theta = 2.0f * RAXTER_PI * u2;
    
    vec3 local = (vec3){r * cosf(theta), r * sinf(theta), sqrtf(1.0f - u1)};
    return vec3_add(vec3_add(vec3_scale(tangent, local.x), vec3_scale(bitangent, local.y)), vec3_scale(normal, local.z));
}

bool light_transport_create(light_transport_renderer* renderer, u32 width, u32 height, u32 spectral_samples, u32 max_bounces) {
    memset(renderer, 0, sizeof(light_transport_renderer));
    renderer->width = width;
    renderer->height = height;
    renderer->spectral_samples = RAXTER_CLAMP(spectral_samples, 1, RAXTER_SPECTRUM_SAMPLES);
    renderer->spp = 0;
    renderer->exposure = 1.0f;
    
    u32 pixel_count = width * height;
    renderer->framebuffer = calloc(pixel_count, sizeof(spectral_power));
    if (!renderer->framebuffer) return false;
    
    for (u32 i = 0; i < pixel_count; i++) {
        renderer->framebuffer[i] = spectrum_make_uniform(0.0f, 380.0f, 780.0f);
    }
    
    return true;
}

void light_transport_destroy(light_transport_renderer* renderer) {
    if (renderer->framebuffer) free(renderer->framebuffer);
    memset(renderer, 0, sizeof(light_transport_renderer));
}

void light_transport_add_source(light_transport_renderer* renderer, const light_source* source) {
    (void)renderer; (void)source;
}

void light_transport_set_config(light_transport_renderer* renderer, const light_transport_config* config) {
    (void)renderer; (void)config;
}

void light_transport_set_exposure(light_transport_renderer* renderer, f32 exposure) {
    renderer->exposure = exposure;
}

spectral_power light_transport_eval_emission(const light_source* source, vec3 direction) {
    (void)direction;
    return spectrum_make_blackbody(6500.0f);
}

bool light_transport_sample_bsdf(const surface_interaction* si, vec3 wo, vec3* wi, spectral_power* bsdf_val, f32* pdf) {
    *wi = random_hemisphere(si->normal);
    *bsdf_val = spectrum_make_uniform(1.0f / RAXTER_PI, 380.0f, 780.0f);
    *pdf = 1.0f / (2.0f * RAXTER_PI);
    return true;
}

f32 light_transport_pdf_bsdf(const surface_interaction* si, vec3 wo, vec3 wi) {
    (void)si; (void)wo; (void)wi;
    return 1.0f / (2.0f * RAXTER_PI);
}

spectral_power light_transport_eval_bsdf(const surface_interaction* si, vec3 wo, vec3 wi) {
    (void)si; (void)wo; (void)wi;
    return spectrum_make_uniform(1.0f / RAXTER_PI, 380.0f, 780.0f);
}

bool light_transport_intersect_scene(vec3 origin, vec3 direction, f32* t, surface_interaction* si) {
    (void)origin; (void)direction; (void)t; (void)si;
    return false;
}

spectral_power light_transport_shade(const surface_interaction* si, vec3 wo, light_transport_renderer* renderer, u32 depth) {
    (void)si; (void)wo; (void)renderer; (void)depth;
    return spectrum_make_uniform(0.0f, 380.0f, 780.0f);
}

void light_transport_trace_paths(light_transport_renderer* renderer, u32 paths_per_pixel) {
    u32 pixel_count = renderer->width * renderer->height;
    
    for (u32 pixel_idx = 0; pixel_idx < pixel_count; pixel_idx++) {
        u32 x = pixel_idx % renderer->width;
        u32 y = pixel_idx / renderer->width;
        
        for (u32 s = 0; s < paths_per_pixel; s++) {
            f32 u = (f32)(x + pcg32_random_float()) / (f32)renderer->width;
            f32 v = (f32)(y + pcg32_random_float()) / (f32)renderer->height;
            
            vec3 ray_dir = vec3_normalize((vec3){u * 2.0f - 1.0f, v * 2.0f - 1.0f, -1.0f});
            vec3 ray_origin = (vec3){0, 0, 0};
            
            spectral_power throughput = spectrum_make_uniform(1.0f, 380.0f, 780.0f);
            
            for (u32 bounce = 0; bounce < 8; bounce++) {
                surface_interaction si;
                f32 t;
                if (!light_transport_intersect_scene(ray_origin, ray_dir, &t, &si)) {
                    break;
                }
                
                vec3 hit_pos = vec3_add(ray_origin, vec3_scale(ray_dir, t));
                
                spectral_power emission = light_transport_eval_emission(NULL, ray_dir);
                renderer->framebuffer[pixel_idx] = spectrum_add(renderer->framebuffer[pixel_idx], 
                    spectrum_mul(throughput, emission));
                
                vec3 wi;
                spectral_power bsdf_val;
                f32 pdf;
                if (!light_transport_sample_bsdf(&si, ray_dir, &wi, &bsdf_val, &pdf)) {
                    break;
                }
                
                throughput = spectrum_mul(throughput, bsdf_val);
                throughput = spectrum_scale(throughput, 1.0f / pdf);
                
                ray_origin = vec3_add(hit_pos, vec3_scale(si.normal, 1e-4f));
                ray_dir = wi;
            }
        }
    }
    
    renderer->spp += paths_per_pixel;
}

void light_transport_resolve(light_transport_renderer* renderer, vec4* output) {
    u32 pixel_count = renderer->width * renderer->height;
    
    for (u32 i = 0; i < pixel_count; i++) {
        spectral_power s = renderer->framebuffer[i];
        s = spectrum_scale(s, 1.0f / (f32)RAXTER_MAX(renderer->spp, 1));
        s = spectrum_scale(s, renderer->exposure);
        
        vec3 rgb = spectrum_to_srgb(s);
        rgb = vec3_scale(rgb, 1.0f / (1.0f + vec3_length(rgb)));
        
        output[i] = (vec4){rgb.x, rgb.y, rgb.z, 1.0f};
    }
}