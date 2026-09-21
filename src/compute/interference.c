#include "interference.h"
#include "core/vk_memory.h"
#include "core/vk_pipeline.h"
#include "core/vk_debug.h"
#include "core/vk_sync.h"
#include <string.h>

static const char* SHADER_PATH = "shaders/interference.comp.spv";

bool interference_create(vk_context* ctx, interference_renderer* renderer, u32 width, u32 height) {
    memset(renderer, 0, sizeof(interference_renderer));
    renderer->width = width;
    renderer->height = height;
    
    VkExtent3D extent = {width, height, 1};
    if (!vk_image_create(ctx, &renderer->output_image, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
                         extent, 1, 1, VK_SAMPLE_COUNT_1_BIT,
                         VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        return false;
    }
    
    if (!vk_image_view_create(ctx, &renderer->output_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT)) {
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDescriptorSetLayoutBinding bindings[] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
    };
    
    if (!vk_descriptor_set_layout_create(ctx, &renderer->set_layout, bindings, 2)) {
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RAXTER_MAX_FRAMES_IN_FLIGHT},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, RAXTER_MAX_FRAMES_IN_FLIGHT},
    };
    
    if (!vk_descriptor_pool_create(ctx, &renderer->desc_pool, RAXTER_MAX_FRAMES_IN_FLIGHT, pool_sizes, 2)) {
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    VkDescriptorSetLayout layouts[RAXTER_MAX_FRAMES_IN_FLIGHT];
    for (u32 i = 0; i < RAXTER_MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = renderer->set_layout;
    
    if (!vk_descriptor_sets_allocate(ctx, renderer->desc_pool, layouts, RAXTER_MAX_FRAMES_IN_FLIGHT, renderer->desc_sets, RAXTER_MAX_FRAMES_IN_FLIGHT)) {
        vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    vk_push_constants push = {0};
    push.ranges[0] = (VkPushConstantRange){
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(interference_params),
    };
    push.range_count = 1;
    
    if (!vk_compute_pipeline_create(ctx, &renderer->pipeline, SHADER_PATH, &renderer->set_layout, 1, &push)) {
        vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
        vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
        vk_image_destroy(ctx, &renderer->output_image);
        return false;
    }
    
    return true;
}

void interference_destroy(vk_context* ctx, interference_renderer* renderer) {
    vk_compute_pipeline_destroy(ctx, &renderer->pipeline);
    vkDestroyDescriptorPool(ctx->device, renderer->desc_pool, NULL);
    vk_descriptor_set_layout_destroy(ctx, renderer->set_layout);
    vk_image_destroy(ctx, &renderer->output_image);
    memset(renderer, 0, sizeof(interference_renderer));
}

void interference_set_params(interference_renderer* renderer, const interference_params* params) {
    (void)renderer; (void)params;
}

void interference_render(vk_context* ctx, interference_renderer* renderer, VkCommandBuffer cmd, u32 frame_index) {
    vk_cmd_begin_label(ctx, cmd, "Interference", (float[4]){1.0f, 0.4f, 0.2f, 1.0f});
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->pipeline.pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->pipeline.layout, 0, 1,
                            &renderer->desc_sets[frame_index], 0, NULL);
    
    u32 group_x = (renderer->width + 15) / 16;
    u32 group_y = (renderer->height + 15) / 16;
    vkCmdDispatch(cmd, group_x, group_y, 1);
    
    vk_cmd_end_label(ctx, cmd);
}