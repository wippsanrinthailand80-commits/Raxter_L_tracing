#ifndef RAXTER_CASIMIR_EFFECT_H
#define RAXTER_CASIMIR_EFFECT_H

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
} casimir_effect_renderer;

typedef struct {
    f32 time;
    f32 plate_separation;
    f32 plate_area;
    f32 temperature;
    int geometry_type;
    f32 curvature;
    int material_model;
} casimir_effect_params;

bool casimir_effect_create(vk_context* ctx, casimir_effect_renderer* renderer, u32 width, u32 height);
void casimir_effect_destroy(vk_context* ctx, casimir_effect_renderer* renderer);
void casimir_effect_set_params(casimir_effect_renderer* renderer, const casimir_effect_params* params);
void casimir_effect_render(vk_context* ctx, casimir_effect_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif