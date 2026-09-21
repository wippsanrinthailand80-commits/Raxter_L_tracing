#ifndef RAXTER_VK_PIPELINE_H
#define RAXTER_VK_PIPELINE_H

#include "vk_core.h"

#define RAXTER_MAX_DESCRIPTOR_SETS 4
#define RAXTER_MAX_BINDINGS 16
#define RAXTER_MAX_PUSH_CONSTANTS 4

typedef struct {
    VkPipelineLayout layout;
    VkPipeline pipeline;
    VkShaderModule* modules;
    u32 module_count;
} vk_compute_pipeline;

typedef struct {
    VkPipelineLayout layout;
    VkPipeline pipeline;
    VkShaderModule* modules;
    u32 module_count;
    VkRenderPass render_pass;
    u32 subpass;
} vk_graphics_pipeline;

typedef struct {
    VkDescriptorSetLayout layouts[RAXTER_MAX_DESCRIPTOR_SETS];
    u32 layout_count;
    VkDescriptorPool pool;
    VkDescriptorSet sets[RAXTER_MAX_FRAMES_IN_FLIGHT];
} vk_descriptor_set_manager;

typedef struct {
    VkPushConstantRange ranges[RAXTER_MAX_PUSH_CONSTANTS];
    u32 range_count;
} vk_push_constants;

bool vk_shader_module_create(vk_context* ctx, VkShaderModule* module, const char* spv_path);
void vk_shader_module_destroy(vk_context* ctx, VkShaderModule module);

bool vk_compute_pipeline_create(vk_context* ctx, vk_compute_pipeline* pipe, 
                                const char* spv_path, VkDescriptorSetLayout* layouts, u32 layout_count,
                                vk_push_constants* push);
void vk_compute_pipeline_destroy(vk_context* ctx, vk_compute_pipeline* pipe);

bool vk_descriptor_pool_create(vk_context* ctx, VkDescriptorPool* pool, u32 max_sets, 
                               VkDescriptorPoolSize* pool_sizes, u32 pool_size_count);
void vk_descriptor_pool_destroy(vk_context* ctx, VkDescriptorPool pool);

bool vk_descriptor_set_layout_create(vk_context* ctx, VkDescriptorSetLayout* layout,
                                     VkDescriptorSetLayoutBinding* bindings, u32 binding_count);
void vk_descriptor_set_layout_destroy(vk_context* ctx, VkDescriptorSetLayout layout);

bool vk_descriptor_sets_allocate(vk_context* ctx, VkDescriptorPool pool, 
                                 VkDescriptorSetLayout* layouts, u32 layout_count,
                                 VkDescriptorSet* sets, u32 set_count);

void vk_descriptor_set_write_buffer(vk_context* ctx, VkDescriptorSet set, u32 binding, 
                                    VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
void vk_descriptor_set_write_image(vk_context* ctx, VkDescriptorSet set, u32 binding,
                                   VkDescriptorType type, VkImageView view, VkSampler sampler, VkImageLayout layout);
void vk_descriptor_set_write_acceleration_structure(vk_context* ctx, VkDescriptorSet set, u32 binding,
                                                     VkAccelerationStructureKHR* structures, u32 count);

bool vk_pipeline_layout_create(vk_context* ctx, VkPipelineLayout* layout,
                               VkDescriptorSetLayout* set_layouts, u32 set_layout_count,
                               vk_push_constants* push);
void vk_pipeline_layout_destroy(vk_context* ctx, VkPipelineLayout layout);

#endif