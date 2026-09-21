#ifndef RAXTER_DIFFRACTION_H
#define RAXTER_DIFFRACTION_H

#include "core/vk_core.h"
#include "core/vk_pipeline.h"
#include "core/vk_memory.h"
#include "core/raxter_types.h"

typedef struct {
    vk_compute_pipeline pipeline;
    vk_image output_image;
    VkDescriptorSetLayout set_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_sets[RAXTER_MAX_FRAMES_IN_FLIGHT];
    u32 width, height;
} diffraction_renderer;

typedef struct {
    u32 num_apertures;
    f32 screen_distance;
    f32 wavelength_nm;
    f32 time;
    int pattern_type;
} diffraction_params;

bool diffraction_create(vk_context* ctx, diffraction_renderer* renderer, u32 width, u32 height);
void diffraction_destroy(vk_context* ctx, diffraction_renderer* renderer);
void diffraction_set_params(diffraction_renderer* renderer, const diffraction_params* params);
void diffraction_render(vk_context* ctx, diffraction_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif