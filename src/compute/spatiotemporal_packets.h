#ifndef RAXTER_SPATIOTEMPORAL_PACKETS_H
#define RAXTER_SPATIOTEMPORAL_PACKETS_H

#include "core/vk_core.h"
#include "core/vk_pipeline.h"
#include "core/vk_memory.h"
#include "core/raxter_types.h"

typedef struct {
    vk_compute_pipeline pipeline;
    vk_buffer volume_buffer;
    VkDescriptorSetLayout set_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_sets[RAXTER_MAX_FRAMES_IN_FLIGHT];
    u32 volume_dim;
} spatiotemporal_packets_renderer;

typedef struct {
    u32 num_sources;
    f32 time;
    f32 voxel_size;
    f32 propagation_distance;
    f32 temporal_window;
    int packet_type;
    f32 beta;
    f32 alpha;
} spatiotemporal_packets_params;

bool spatiotemporal_packets_create(vk_context* ctx, spatiotemporal_packets_renderer* renderer, u32 volume_dim);
void spatiotemporal_packets_destroy(vk_context* ctx, spatiotemporal_packets_renderer* renderer);
void spatiotemporal_packets_set_params(spatiotemporal_packets_renderer* renderer, const spatiotemporal_packets_params* params);
void spatiotemporal_packets_render(vk_context* ctx, spatiotemporal_packets_renderer* renderer, VkCommandBuffer cmd, u32 frame_index);

#endif