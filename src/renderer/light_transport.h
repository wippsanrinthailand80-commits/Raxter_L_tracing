#ifndef RAXTER_LIGHT_TRANSPORT_H
#define RAXTER_LIGHT_TRANSPORT_H

#include "core/raxter_types.h"
#include "math/wave.h"
#include "math/spectrum.h"

#define RAXTER_MAX_LIGHT_SOURCES 64
#define RAXTER_MAX_BOUNCES 8
#define RAXTER_MAX_PATHS 1024

typedef struct {
    vec3 position;
    vec3 normal;
    vec3 emission;
    vec3 albedo;
    f32 roughness;
    f32 metallic;
    f32 ior;
    material_optical optical;
} surface_interaction;

typedef struct {
    vec3 origin;
    vec3 direction;
    spectral_power throughput;
    u32 depth;
    u32 max_depth;
    f32 pdf;
    bool is_delta;
} light_path;

typedef struct {
    light_source sources[RAXTER_MAX_LIGHT_SOURCES];
    u32 source_count;
    u32 max_bounces;
    f32 wavelength_min;
    f32 wavelength_max;
    u32 spectral_samples;
    bool enable_interference;
    bool enable_diffraction;
    bool enable_polarization;
    bool enable_volume_scattering;
} light_transport_config;

typedef struct {
    spectral_power* framebuffer;
    u32 width;
    u32 height;
    u32 spp;
    u32 spectral_samples;
    f32 exposure;
} light_transport_renderer;

bool light_transport_create(light_transport_renderer* renderer, u32 width, u32 height, u32 spectral_samples, u32 max_bounces);
void light_transport_destroy(light_transport_renderer* renderer);

void light_transport_add_source(light_transport_renderer* renderer, const light_source* source);
void light_transport_set_config(light_transport_renderer* renderer, const light_transport_config* config);

void light_transport_trace_paths(light_transport_renderer* renderer, u32 paths_per_pixel);
void light_transport_resolve(light_transport_renderer* renderer, vec4* output);

spectral_power light_transport_eval_emission(const light_source* source, vec3 direction);
bool light_transport_sample_bsdf(const surface_interaction* si, vec3 wo, vec3* wi, spectral_power* bsdf_val, f32* pdf);
f32 light_transport_pdf_bsdf(const surface_interaction* si, vec3 wo, vec3 wi);
spectral_power light_transport_eval_bsdf(const surface_interaction* si, vec3 wo, vec3 wi);

bool light_transport_intersect_scene(vec3 origin, vec3 direction, f32* t, surface_interaction* si);
spectral_power light_transport_shade(const surface_interaction* si, vec3 wo, light_transport_renderer* renderer, u32 depth);

void light_transport_set_exposure(light_transport_renderer* renderer, f32 exposure);

#endif