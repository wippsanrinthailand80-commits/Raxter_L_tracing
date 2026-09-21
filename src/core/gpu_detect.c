#include "gpu_detect.h"
#include <string.h>
#include <stdio.h>

static gpu_tier classify_by_vendor_device(uint32_t vendor_id, uint32_t device_id, VkPhysicalDeviceType type) {
    switch (vendor_id) {
        case 0x1002: { // AMD
            if (type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) return GPU_TIER_MID;
            if (device_id >= 0x7440 && device_id <= 0x747F) return GPU_TIER_ULTRA; // RDNA3 (7900)
            if (device_id >= 0x73A0 && device_id <= 0x73FF) return GPU_TIER_HIGH;   // RDNA2 (6800/6900)
            if (device_id >= 0x7310 && device_id <= 0x735F) return GPU_TIER_HIGH;   // RDNA1 (5700)
            return GPU_TIER_MID;
        }
        case 0x10DE: { // NVIDIA
            if (type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) return GPU_TIER_MID;
            if (device_id >= 0x2680 && device_id <= 0x26FF) return GPU_TIER_ULTRA; // Ada (4090/4080)
            if (device_id >= 0x2480 && device_id <= 0x24FF) return GPU_TIER_HIGH;   // Ampere (3080/3090)
            if (device_id >= 0x1E00 && device_id <= 0x1EFF) return GPU_TIER_HIGH;   // Turing (2080)
            return GPU_TIER_MID;
        }
        case 0x8086: { // Intel
            if (type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) return GPU_TIER_LOW;
            if (device_id >= 0x56A0) return GPU_TIER_MID; // Arc
            return GPU_TIER_LOW;
        }
        case 0x13B5: { // ARM Mali
            if (device_id >= 0x6200) return GPU_TIER_HIGH;  // G715/G720
            if (device_id >= 0x6000) return GPU_TIER_MID;   // G78/G710
            if (device_id >= 0x5000) return GPU_TIER_LOW;   // G57/G68
            return GPU_TIER_LOW;
        }
        case 0x5143: { // Qualcomm Adreno
            if (device_id >= 0x740) return GPU_TIER_HIGH;   // Adreno 740/750
            if (device_id >= 0x640) return GPU_TIER_MID;    // Adreno 640/650/660
            if (device_id >= 0x600) return GPU_TIER_LOW;    // Adreno 618/619
            return GPU_TIER_LOW;
        }
        case 0x144D: { // Samsung Xclipse (mRDNA)
            return GPU_TIER_MID;
        }
        case 0x1010: { // Imagination
            return GPU_TIER_LOW;
        }
        case 0x1000: { // Vivante
            return GPU_TIER_LOW;
        }
        default: {
            if (type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) return GPU_TIER_HIGH;
            if (type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) return GPU_TIER_LOW;
            return GPU_TIER_MID;
        }
    }
}

gpu_tier gpu_detect_tier(vk_context* ctx) {
    if (!ctx || !ctx->physical_device) return GPU_TIER_UNKNOWN;
    
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(ctx->physical_device, &props);
    
    // Check for software renderer
    if (strstr(props.deviceName, "llvmpipe") || 
        strstr(props.deviceName, "software") ||
        (props.vendorID == 0x10005 && props.deviceID == 0x0000)) {
        fprintf(stderr, "\n============================================================\n");
        fprintf(stderr, "WARNING: Running on SOFTWARE RENDERER (llvmpipe)!\n");
        fprintf(stderr, "Device: %s\n", props.deviceName);
        fprintf(stderr, "Performance will be 10-100x SLOWER than hardware GPU.\n");
        fprintf(stderr, "Install proper Vulkan ICD drivers for your GPU:\n");
        fprintf(stderr, "  AMD:    sudo apt install mesa-vulkan-drivers\n");
        fprintf(stderr, "  NVIDIA: sudo apt install nvidia-driver-550\n");
        fprintf(stderr, "  Intel:  sudo apt install mesa-vulkan-drivers\n");
        fprintf(stderr, "  ARM:    Install vendor Mali/Adreno Vulkan ICD\n");
        fprintf(stderr, "See DRIVERS.md for complete installation guide.\n");
        fprintf(stderr, "============================================================\n\n");
    }
    
    gpu_tier tier = classify_by_vendor_device(props.vendorID, props.deviceID, props.deviceType);
    
    printf("GPU Detected: %s (Vendor: 0x%04X, Device: 0x%04X, Type: %d) -> Tier: %s\n",
           props.deviceName, props.vendorID, props.deviceID, props.deviceType,
           gpu_tier_name(tier));
    
    return tier;
}

quality_preset quality_preset_for_tier(gpu_tier tier) {
    switch (tier) {
        case GPU_TIER_LOW:    return QUALITY_PRESET_LOW;
        case GPU_TIER_MID:    return QUALITY_PRESET_MEDIUM;
        case GPU_TIER_HIGH:   return QUALITY_PRESET_HIGH;
        case GPU_TIER_ULTRA:  return QUALITY_PRESET_ULTRA;
        default:              return QUALITY_PRESET_MEDIUM;
    }
}

void quality_settings_apply(quality_preset preset, quality_settings* out) {
    switch (preset) {
        case QUALITY_PRESET_LOW:
            out->max_bounces = 1;
            out->spectral_samples = 4;
            out->volume_dim = 32;
            out->max_oam_charge = 0;
            out->enable_volume_propagation = false;
            out->enable_four_wave_mixing = false;
            out->enable_casimir = false;
            out->enable_ghost_imaging = false;
            out->render_scale = 0.5f;
            break;
        case QUALITY_PRESET_MEDIUM:
            out->max_bounces = 2;
            out->spectral_samples = 8;
            out->volume_dim = 64;
            out->max_oam_charge = 2;
            out->enable_volume_propagation = true;
            out->enable_four_wave_mixing = false;
            out->enable_casimir = false;
            out->enable_ghost_imaging = false;
            out->render_scale = 0.75f;
            break;
        case QUALITY_PRESET_HIGH:
            out->max_bounces = 4;
            out->spectral_samples = 16;
            out->volume_dim = 64;
            out->max_oam_charge = 4;
            out->enable_volume_propagation = true;
            out->enable_four_wave_mixing = true;
            out->enable_casimir = true;
            out->enable_ghost_imaging = false;
            out->render_scale = 1.0f;
            break;
        case QUALITY_PRESET_ULTRA:
            out->max_bounces = 8;
            out->spectral_samples = 32;
            out->volume_dim = 128;
            out->max_oam_charge = 8;
            out->enable_volume_propagation = true;
            out->enable_four_wave_mixing = true;
            out->enable_casimir = true;
            out->enable_ghost_imaging = true;
            out->render_scale = 1.0f;
            break;
    }
}

const char* gpu_tier_name(gpu_tier tier) {
    switch (tier) {
        case GPU_TIER_LOW:    return "Low";
        case GPU_TIER_MID:    return "Mid";
        case GPU_TIER_HIGH:   return "High";
        case GPU_TIER_ULTRA:  return "Ultra";
        default:              return "Unknown";
    }
}

const char* quality_preset_name(quality_preset preset) {
    switch (preset) {
        case QUALITY_PRESET_LOW:    return "Low";
        case QUALITY_PRESET_MEDIUM: return "Medium";
        case QUALITY_PRESET_HIGH:   return "High";
        case QUALITY_PRESET_ULTRA:  return "Ultra";
        default:                    return "Unknown";
    }
}