#ifndef RAXTER_GHOST_IMAGING_H
#define RAXTER_GHOST_IMAGING_H

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
} ghost_imaging_renderer;

typedef struct {
    u32 num_sources;
    f32 time;
    int protocol_type;
    f32 object_transmission;
    f32 noise_level;
    int correlation_order;
    int measurement_basis;
    f32 bucket_detector_size;
    f32 spatial_resolution;
} ghost_imaging_params;

bool ghost_imaging_create(vk_context* ctx, ghost_imaging_renderer* renderer, u32 width, u32 height);
void ghost_imaging_destroy(vk_context* ctx, ghost_imaging_renderer* renderer);
void ghost_imaging_set_params(ghost_imaging_renderer* renderer, const ghost_imaging_params* params);
void ghost_imaging_render(vk_context* ctx, ghost_imaging_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif