#ifndef RAXTER_INTERFERENCE_H
#define RAXTER_INTERFERENCE_H

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
} interference_renderer;

typedef struct {
    u32 num_slits;
    f32 slit_separation;
    f32 screen_distance;
    f32 wavelength_nm;
    f32 time;
} interference_params;

bool interference_create(vk_context* ctx, interference_renderer* renderer, u32 width, u32 height);
void interference_destroy(vk_context* ctx, interference_renderer* renderer);
void interference_set_params(interference_renderer* renderer, const interference_params* params);
void interference_render(vk_context* ctx, interference_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif