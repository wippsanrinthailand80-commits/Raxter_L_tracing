#include "spatiotemporal_packets.h"
#include "core/vk_memory.h"
#include "core/vk_pipeline.h"
#include "core/vk_debug.h"
#include <string.h>

static const char* SHADER_PATH = "shaders/spatiotemporal_packets.comp.spv";

bool spatiotemporal_packets_create(vk_context* ctx, spatiotemporal_packets_renderer* renderer, u32 volume_dim) {
    memset(renderer, 0, sizeof(spatiotemporal_packets_renderer));
    renderer->volume_dim = volume_dim;
    
    u32 volume_size = volume_dim * volume_dim * volume_dim;
    VkDeviceSize volume_buffer_size = sizeof(vec4) * volume_size;
    if (!vk_buffer_create(ctx, &renderer->volume_buffer, volume_buffer_size,
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        return false;
    }
    
    VkDescriptorSetLayoutBinding bindings[] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
    };
    
    if (!vk_descriptor_set_layout_create(ctx, &renderer->set_layout, bindings, 2)) {
        vk_buffer_destroy(ctx, &renderer->volume_buffer);
        return false;
    }
    
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RAXTER_MAX_FRAMES_IN_FLIGHT},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, RAXTER_MAX_FRAMES_IN_FLIGHT},
    };
    
    if (!vk_descriptor_pool_create(ctx, &renderer->desc_pool, RAXTER_MAX_FRAMES_IN_FLIGHT, pool_sizes, 2)) {
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_buffer_destroy(ctx, &renderer->volume_buffer);
        return false;
    }
    
    VkDescriptorSetLayout layouts[RAXTER_MAX_FRAMES_IN_FLIGHT];
    for (u32 i = 0; i < RAXTER_MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = renderer->set_layout;
    
    if (!vk_descriptor_sets_allocate(ctx, renderer->desc_pool, layouts, RAXTER_MAX_FRAMES_IN_FLIGHT, renderer->desc_sets, RAXTER_MAX_FRAMES_IN_FLIGHT)) {
        vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_buffer_destroy(ctx, &renderer->volume_buffer);
        return false;
    }
    
    vk_push_constants push = {0};
    push.ranges[0] = (VkPushConstantRange){
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(spatiotemporal_packets_params),
    };
    push.range_count = 1;
    
    if (!vk_compute_pipeline_create(ctx, &renderer->pipeline, SHADER_PATH, &renderer->set_layout, 1, &push)) {
        vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_buffer_destroy(ctx, &renderer->volume_buffer);
        return false;
    }
    
    return true;
}

void spatiotemporal_packets_destroy(vk_context* ctx, spatiotemporal_packets_renderer* renderer) {
    vk_compute_pipeline_destroy(ctx, &renderer->pipeline);
    vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
    vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
    vk_buffer_destroy(ctx, &renderer->volume_buffer);
    memset(renderer, 0, sizeof(spatiotemporal_packets_renderer));
}

void spatiotemporal_packets_set_params(spatiotemporal_packets_renderer* renderer, const spatiotemporal_packets_params* params) {
    (void)renderer; (void)params;
}

void spatiotemporal_packets_render(vk_context* ctx, spatiotemporal_packets_renderer* renderer, VkCommandBuffer cmd, u32 frame_index) {
    vk_cmd_begin_label(ctx, cmd, "Spatiotemporal Packets", (float[4]){0.2f, 0.8f, 0.4f, 1.0f});
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->pipeline.pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->pipeline.layout, 0, 1,
                            &renderer->desc_sets[frame_index], 0, NULL);
    
    u32 group_size = 8;
    u32 group_x = (renderer->volume_dim + group_size - 1) / group_size;
    u32 group_y = (renderer->volume_dim + group_size - 1) / group_size;
    u32 group_z = (renderer->volume_dim + 3) / 4;
    vkCmdDispatch(cmd, group_x, group_y, group_z);
    
    vk_cmd_end_label(ctx, cmd);
}