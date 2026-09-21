#ifndef RAXTER_GPU_DETECT_H
#define RAXTER_GPU_DETECT_H

#include "vk_core.h"
#include "raxter_types.h"

typedef enum {
    GPU_TIER_UNKNOWN = 0,
    GPU_TIER_LOW = 1,      // Integrated, older mobile (Mali-G57, Adreno 610)
    GPU_TIER_MID = 2,      // Mid mobile/desktop (Mali-G78, Adreno 640, RX 5500)
    GPU_TIER_HIGH = 3,     // Flagship mobile/desktop (Mali-G715, Adreno 740, RX 6700)
    GPU_TIER_ULTRA = 4,    // Top desktop (RX 7900 XTX, RTX 4090)
} gpu_tier;

typedef enum {
    QUALITY_PRESET_LOW = 0,
    QUALITY_PRESET_MEDIUM = 1,
    QUALITY_PRESET_HIGH = 2,
    QUALITY_PRESET_ULTRA = 3,
} quality_preset;

typedef struct {
    uint32_t max_bounces;
    uint32_t spectral_samples;
    uint32_t volume_dim;
    uint32_t max_oam_charge;
    bool enable_volume_propagation;
    bool enable_four_wave_mixing;
    bool enable_casimir;
    bool enable_ghost_imaging;
    float render_scale;
} quality_settings;

gpu_tier gpu_detect_tier(vk_context* ctx);
quality_preset quality_preset_for_tier(gpu_tier tier);
void quality_settings_apply(quality_preset preset, quality_settings* out);
const char* gpu_tier_name(gpu_tier tier);
const char* quality_preset_name(quality_preset preset);

#endif