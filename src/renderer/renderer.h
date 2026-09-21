#ifndef RAXTER_RENDERER_H
#define RAXTER_RENDERER_H

#include "../core/vk_core.h"
#include "../core/vk_swapchain.h"
#include "../core/vk_command_pool.h"
#include "../core/vk_sync.h"
#include "../compute/wave_optics.h"
#include "../compute/interference.h"
#include "../compute/diffraction.h"
#include "../compute/polarization.h"
#include "../compute/light_propagation.h"
#include "../compute/quantum_coherence.h"
#include "../compute/oam_beams.h"
#include "../compute/spatiotemporal_packets.h"
#include "../compute/four_wave_mixing.h"
#include "../compute/casimir_effect.h"
#include "../compute/ghost_imaging.h"
#include "../core/raxter_types.h"
#include "../core/gpu_detect.h"

typedef enum {
    RAXTER_RENDER_MODE_WAVE_OPTICS = 0,
    RAXTER_RENDER_MODE_INTERFERENCE = 1,
    RAXTER_RENDER_MODE_DIFFRACTION = 2,
    RAXTER_RENDER_MODE_POLARIZATION = 3,
    RAXTER_RENDER_MODE_LIGHT_PROPAGATION = 4,
    RAXTER_RENDER_MODE_QUANTUM_COHERENCE = 5,
    RAXTER_RENDER_MODE_OAM_BEAMS = 6,
    RAXTER_RENDER_MODE_SPATIOTEMPORAL_PACKETS = 7,
    RAXTER_RENDER_MODE_FOUR_WAVE_MIXING = 8,
    RAXTER_RENDER_MODE_CASIMIR_EFFECT = 9,
    RAXTER_RENDER_MODE_GHOST_IMAGING = 10,
} raxter_render_mode;

typedef struct {
    vk_context vk;
    vk_swapchain swapchain;
    vk_command_pool graphics_pool;
    vk_command_pool compute_pool;
    vk_sync_objects sync;
    vk_debug debug;
    
    wave_optics_renderer wave_optics;
    interference_renderer interference;
    diffraction_renderer diffraction;
    polarization_renderer polarization;
    light_propagation_renderer light_propagation;
    quantum_coherence_renderer quantum_coherence;
    oam_beams_renderer oam_beams;
    spatiotemporal_packets_renderer spatiotemporal_packets;
    four_wave_mixing_renderer four_wave_mixing;
    casimir_effect_renderer casimir_effect;
    ghost_imaging_renderer ghost_imaging;
    
    raxter_render_mode current_mode;
    u32 width, height;
    u32 max_bounces;
    bool validation_enabled;
    f32 frame_time;
    u64 frame_count;
    
    gpu_tier gpu_tier;
    quality_preset quality_preset;
    quality_settings quality;
    bool auto_quality;
} raxter_renderer;

bool raxter_renderer_create(raxter_renderer* renderer, const char* app_name, u32 width, u32 height, u32 max_bounces, bool validation);
void raxter_renderer_destroy(raxter_renderer* renderer);

bool raxter_renderer_begin_frame(raxter_renderer* renderer);
void raxter_renderer_end_frame(raxter_renderer* renderer);

void raxter_renderer_set_mode(raxter_renderer* renderer, raxter_render_mode mode);
void raxter_renderer_resize(raxter_renderer* renderer, u32 width, u32 height);

void raxter_renderer_update(raxter_renderer* renderer, f32 delta_time);

void raxter_renderer_auto_quality(raxter_renderer* renderer);
void raxter_renderer_set_quality(raxter_renderer* renderer, quality_preset preset);
void raxter_renderer_apply_quality(raxter_renderer* renderer);

#endif