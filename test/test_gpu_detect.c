#include "core/vk_core.h"
#include "core/gpu_detect.h"
#include <stdio.h>

int main() {
    printf("=== RAXTER GPU Detection & Quality Preset Test ===\n\n");
    
    vk_context ctx;
    if (!vk_context_create(&ctx, "RAXTER GPU Detect Test", false)) {
        fprintf(stderr, "Failed to create Vulkan context\n");
        return 1;
    }
    
    gpu_tier tier = gpu_detect_tier(&ctx);
    printf("\nDetected GPU Tier: %s\n", gpu_tier_name(tier));
    
    quality_preset preset = quality_preset_for_tier(tier);
    printf("Recommended Quality Preset: %s\n\n", quality_preset_name(preset));
    
    quality_settings settings;
    quality_settings_apply(preset, &settings);
    
    printf("Quality Settings Applied:\n");
    printf("  Max Bounces: %u\n", settings.max_bounces);
    printf("  Spectral Samples: %u\n", settings.spectral_samples);
    printf("  Volume Dimension: %u\n", settings.volume_dim);
    printf("  Max OAM Charge: %u\n", settings.max_oam_charge);
    printf("  Volume Propagation: %s\n", settings.enable_volume_propagation ? "ON" : "OFF");
    printf("  Four Wave Mixing: %s\n", settings.enable_four_wave_mixing ? "ON" : "OFF");
    printf("  Casimir Effect: %s\n", settings.enable_casimir ? "ON" : "OFF");
    printf("  Ghost Imaging: %s\n", settings.enable_ghost_imaging ? "ON" : "OFF");
    printf("  Render Scale: %.2f\n", settings.render_scale);
    
    printf("\n=== Testing Manual Quality Override ===\n");
    for (int i = 0; i < 4; i++) {
        quality_preset p = (quality_preset)i;
        quality_settings s;
        quality_settings_apply(p, &s);
        printf("%s: bounces=%u, spectral=%u, volume=%u, oam=%u, scale=%.2f\n",
               quality_preset_name(p), s.max_bounces, s.spectral_samples, 
               s.volume_dim, s.max_oam_charge, s.render_scale);
    }
    
    vk_context_destroy(&ctx);
    printf("\n=== Test Complete ===\n");
    return 0;
}