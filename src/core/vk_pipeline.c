#include "vk_pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t read_file(const char* path, void** data) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *data = malloc(size);
    fread(*data, 1, size, f);
    fclose(f);
    return size;
}

bool vk_shader_module_create(vk_context* ctx, VkShaderModule* module, const char* spv_path) {
    void* code;
    size_t size = read_file(spv_path, &code);
    if (size == 0) {
        fprintf(stderr, "Failed to read shader: %s\n", spv_path);
        return false;
    }
    
    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = code,
    };
    
    VkResult result = vkCreateShaderModule(ctx->device, &info, NULL, module);
    free(code);
    return result == VK_SUCCESS;
}

void vk_shader_module_destroy(vk_context* ctx, VkShaderModule module) {
    if (module) vkDestroyShaderModule(ctx->device, module, NULL);
}

bool vk_pipeline_layout_create(vk_context* ctx, VkPipelineLayout* layout,
                               VkDescriptorSetLayout* set_layouts, u32 set_layout_count,
                               vk_push_constants* push) {
    VkPipelineLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = set_layout_count,
        .pSetLayouts = set_layouts,
        .pushConstantRangeCount = push ? push->range_count : 0,
        .pPushConstantRanges = push ? push->ranges : NULL,
    };
    return vkCreatePipelineLayout(ctx->device, &info, NULL, layout) == VK_SUCCESS;
}

void vk_pipeline_layout_destroy(vk_context* ctx, VkPipelineLayout layout) {
    if (layout) vkDestroyPipelineLayout(ctx->device, layout, NULL);
}

bool vk_compute_pipeline_create(vk_context* ctx, vk_compute_pipeline* pipe,
                                const char* spv_path, VkDescriptorSetLayout* layouts, u32 layout_count,
                                vk_push_constants* push) {
    memset(pipe, 0, sizeof(vk_compute_pipeline));
    
    if (!vk_shader_module_create(ctx, &pipe->modules[0], spv_path)) return false;
    pipe->module_count = 1;
    
    if (!vk_pipeline_layout_create(ctx, &pipe->layout, layouts, layout_count, push)) {
        vk_shader_module_destroy(ctx, pipe->modules[0]);
        return false;
    }
    
    VkComputePipelineCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = pipe->modules[0],
            .pName = "main",
        },
        .layout = pipe->layout,
    };
    
    VkResult result = vkCreateComputePipelines(ctx->device, VK_NULL_HANDLE, 1, &info, NULL, &pipe->pipeline);
    if (result != VK_SUCCESS) {
        vk_pipeline_layout_destroy(ctx, pipe->layout);
        vk_shader_module_destroy(ctx, pipe->modules[0]);
        return false;
    }
    return true;
}

void vk_compute_pipeline_destroy(vk_context* ctx, vk_compute_pipeline* pipe) {
    if (pipe->pipeline) vkDestroyPipeline(ctx->device, pipe->pipeline, NULL);
    if (pipe->layout) vkDestroyPipelineLayout(ctx->device, pipe->layout, NULL);
    for (u32 i = 0; i < pipe->module_count; i++) {
        if (pipe->modules[i]) vkDestroyShaderModule(ctx->device, pipe->modules[i], NULL);
    }
    memset(pipe, 0, sizeof(vk_compute_pipeline));
}

bool vk_descriptor_pool_create(vk_context* ctx, VkDescriptorPool* pool, u32 max_sets,
                               VkDescriptorPoolSize* pool_sizes, u32 pool_size_count) {
    VkDescriptorPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = max_sets,
        .poolSizeCount = pool_size_count,
        .pPoolSizes = pool_sizes,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    };
    return vkCreateDescriptorPool(ctx->device, &info, NULL, pool) == VK_SUCCESS;
}

void vk_descriptor_pool_destroy(vk_context* ctx, VkDescriptorPool pool) {
    if (pool) vkDestroyDescriptorPool(ctx->device, pool, NULL);
}

bool vk_descriptor_set_layout_create(vk_context* ctx, VkDescriptorSetLayout* layout,
                                     VkDescriptorSetLayoutBinding* bindings, u32 binding_count) {
    VkDescriptorSetLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = binding_count,
        .pBindings = bindings,
    };
    return vkCreateDescriptorSetLayout(ctx->device, &info, NULL, layout) == VK_SUCCESS;
}

void vk_descriptor_set_layout_destroy(vk_context* ctx, VkDescriptorSetLayout layout) {
    if (layout) vkDestroyDescriptorSetLayout(ctx->device, layout, NULL);
}

bool vk_descriptor_sets_allocate(vk_context* ctx, VkDescriptorPool pool,
                                 VkDescriptorSetLayout* layouts, u32 layout_count,
                                 VkDescriptorSet* sets, u32 set_count) {
    VkDescriptorSetAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = set_count,
        .pSetLayouts = layouts,
    };
    return vkAllocateDescriptorSets(ctx->device, &info, sets) == VK_SUCCESS;
}

void vk_descriptor_set_write_buffer(vk_context* ctx, VkDescriptorSet set, u32 binding,
                                    VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
    VkDescriptorBufferInfo buf_info = {.buffer = buffer, .offset = offset, .range = range};
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &buf_info,
    };
    vkUpdateDescriptorSets(ctx->device, 1, &write, 0, NULL);
}

void vk_descriptor_set_write_image(vk_context* ctx, VkDescriptorSet set, u32 binding,
                                   VkDescriptorType type, VkImageView view, VkSampler sampler, VkImageLayout layout) {
    VkDescriptorImageInfo img_info = {.sampler = sampler, .imageView = view, .imageLayout = layout};
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &img_info,
    };
    vkUpdateDescriptorSets(ctx->device, 1, &write, 0, NULL);
}

void vk_descriptor_set_write_acceleration_structure(vk_context* ctx, VkDescriptorSet set, u32 binding,
                                                     VkAccelerationStructureKHR* structures, u32 count) {
    VkWriteDescriptorSetAccelerationStructureKHR as_info = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
        .accelerationStructureCount = count,
        .pAccelerationStructures = structures,
    };
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = binding,
        .descriptorCount = count,
        .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        .pNext = &as_info,
    };
    vkUpdateDescriptorSets(ctx->device, 1, &write, 0, NULL);
}