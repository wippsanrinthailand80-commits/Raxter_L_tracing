#ifndef RAXTER_POLARIZATION_H
#define RAXTER_POLARIZATION_H

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
} polarization_renderer;

typedef struct {
    f32 wavelength_nm;
    f32 polarizer_angle;
    f32 analyzer_angle;
    f32 retarder_phase;
    int retarder_type;
    int test_mode;
} polarization_params;

bool polarization_create(vk_context* ctx, polarization_renderer* renderer, u32 width, u32 height);
void polarization_destroy(vk_context* ctx, polarization_renderer* renderer);
void polarization_set_params(polarization_renderer* renderer, const polarization_params* params);
void polarization_render(vk_context* ctx, polarization_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif