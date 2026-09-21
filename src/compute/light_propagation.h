#ifndef RAXTER_LIGHT_PROPAGATION_H
#define RAXTER_LIGHT_PROPAGATION_H

#include "core/vk_core.h"
#include "core/vk_pipeline.h"
#include "core/vk_memory.h"
#include "core/raxter_types.h"
#include "math/wave.h"
#include "math/vec.h"
#include "../renderer/scene.h"

#define RAXTER_MAX_VOLUME_DIM 128

typedef struct {
    vk_compute_pipeline pipeline;
    vk_buffer source_buffer;
    vk_buffer object_buffer;
    vk_buffer material_buffer;
    vk_buffer volume_buffer;
    VkDescriptorSetLayout set_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_sets[RAXTER_MAX_FRAMES_IN_FLIGHT];
    u32 volume_dim;
} light_propagation_renderer;

typedef struct {
    u32 num_sources;
    u32 num_objects;
    u32 max_bounces;
    f32 time;
    f32 voxel_size;
    vec3 volume_origin;
} light_propagation_params;

bool light_propagation_create(vk_context* ctx, light_propagation_renderer* renderer, u32 volume_dim);
void light_propagation_destroy(vk_context* ctx, light_propagation_renderer* renderer);
void light_propagation_set_params(light_propagation_renderer* renderer, const light_propagation_params* params);
void light_propagation_render(vk_context* ctx, light_propagation_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif