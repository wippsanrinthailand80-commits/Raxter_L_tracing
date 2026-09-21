#include "wave_optics.h"
#include "core/vk_memory.h"
#include "core/vk_pipeline.h"
#include "core/vk_descriptor.h"
#include "core/vk_debug.h"
#include "core/vk_sync.h"
#include <string.h>
#include <stdlib.h>

static const char* SHADER_PATH = "shaders/wave_optics.comp.spv";

bool wave_optics_create(vk_context* ctx, wave_optics_renderer* renderer, u32 width, u32 height, u32 max_bounces) {
    memset(renderer, 0, sizeof(wave_optics_renderer));
    renderer->width = width;
    renderer->height = height;
    
    VkExtent3D extent = {width, height, 1};
    if (!vk_image_create(ctx, &renderer->output_image, VK_IMAGE_TYPE_2D, VK_FORMAT_R32G32B32A32_SFLOAT,
                         extent, 1, 1, VK_SAMPLE_COUNT_1_BIT,
                         VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        return false;
    }
    
    if (!vk_image_view_create(ctx, &renderer->output_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT)) {
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDeviceSize source_buffer_size = sizeof(light_source) * RAXTER_MAX_SOURCES + sizeof(u32);
    if (!vk_buffer_create(ctx, &renderer->source_buffer, source_buffer_size,
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDeviceSize material_buffer_size = sizeof(material_optical) * 256;
    if (!vk_buffer_create(ctx, &renderer->material_buffer, material_buffer_size,
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        vk_buffer_destroy(ctx, &renderer->source_buffer);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDescriptorSetLayoutBinding bindings[] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
    };
    
    if (!vk_descriptor_set_layout_create(ctx, &renderer->set_layout, bindings, 4)) {
        vk_buffer_destroy(ctx, &renderer->source_buffer);
        vk_buffer_destroy(ctx, &renderer->material_buffer);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, RAXTER_MAX_FRAMES_IN_FLIGHT * 2},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, RAXTER_MAX_FRAMES_IN_FLIGHT},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RAXTER_MAX_FRAMES_IN_FLIGHT},
    };
    
    if (!vk_descriptor_pool_create(ctx, &renderer->desc_pool, RAXTER_MAX_FRAMES_IN_FLIGHT, pool_sizes, 3)) {
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_buffer_destroy(ctx, &renderer->source_buffer);
        vk_buffer_destroy(ctx, &renderer->material_buffer);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDescriptorSetLayout layouts[RAXTER_MAX_FRAMES_IN_FLIGHT];
    for (u32 i = 0; i < RAXTER_MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = renderer->set_layout;
    
    if (!vk_descriptor_sets_allocate(ctx, renderer->desc_pool, layouts, RAXTER_MAX_FRAMES_IN_FLIGHT, renderer->desc_sets, RAXTER_MAX_FRAMES_IN_FLIGHT)) {
        vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_buffer_destroy(ctx, &renderer->source_buffer);
        vk_buffer_destroy(ctx, &renderer->material_buffer);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    vk_push_constants push = {0};
    push.ranges[0] = (VkPushConstantRange){
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(wave_optics_params),
    };
    push.range_count = 1;
    
    if (!vk_compute_pipeline_create(ctx, &renderer->pipeline, SHADER_PATH, &renderer->set_layout, 1, &push)) {
        vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_buffer_destroy(ctx, &renderer->source_buffer);
        vk_buffer_destroy(ctx, &renderer->material_buffer);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    return true;
}

void wave_optics_destroy(vk_context* ctx, wave_optics_renderer* renderer) {
    vk_compute_pipeline_destroy(ctx, &renderer->pipeline);
    vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
    vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
    vk_buffer_destroy(ctx, &renderer->source_buffer);
    vk_buffer_destroy(ctx, &renderer->material_buffer);
    vk_image_destroy(ctx, &renderer->output_image);
    memset(renderer, 0, sizeof(wave_optics_renderer));
}

void wave_optics_set_sources(wave_optics_renderer* renderer, const light_source* sources, u32 count) {
    // Implementation would upload to GPU buffer
    (void)renderer; (void)sources; (void)count;
}

void wave_optics_set_params(wave_optics_renderer* renderer, const wave_optics_params* params) {
    // Implementation would upload uniform buffer
    (void)renderer; (void)params;
}

void wave_optics_render(vk_context* ctx, wave_optics_renderer* renderer, VkCommandBuffer cmd, u32 frame_index) {
    vk_cmd_begin_label(ctx, cmd, "Wave Optics", (float[4]){0.2f, 0.6f, 1.0f, 1.0f});
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->pipeline.pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->pipeline.layout, 0, 1, 
                            &renderer->desc_sets[frame_index], 0, NULL);
    
    u32 group_x = (renderer->width + 15) / 16;
    u32 group_y = (renderer->height + 15) / 16;
    vkCmdDispatch(cmd, group_x, group_y, 1);
    
    vk_cmd_end_label(ctx, cmd);
}

void wave_optics_resize(wave_optics_renderer* renderer, u32 width, u32 height) {
    (void)renderer; (void)width; (void)height;
}