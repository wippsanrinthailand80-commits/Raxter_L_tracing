#include "vk_pipeline.h"
#include "vk_descriptor.h"
#include <stdlib.h>
#include <string.h>

bool vk_descriptor_manager_create(vk_context* ctx, vk_descriptor_set_manager* mgr,
                                  VkDescriptorSetLayout* layouts, u32 layout_count,
                                  VkDescriptorPoolSize* pool_sizes, u32 pool_size_count) {
    memset(mgr, 0, sizeof(vk_descriptor_set_manager));
    mgr->layout_count = layout_count;
    
    for (u32 i = 0; i < layout_count; i++) {
        mgr->layouts[i] = layouts[i];
    }
    
    if (!vk_descriptor_pool_create(ctx, &mgr->pool, RAXTER_MAX_FRAMES_IN_FLIGHT * layout_count, 
                                    pool_sizes, pool_size_count)) {
        return false;
    }
    
    VkDescriptorSetLayout set_layouts[RAXTER_MAX_FRAMES_IN_FLIGHT * RAXTER_MAX_DESCRIPTOR_SETS];
    for (u32 frame = 0; frame < RAXTER_MAX_FRAMES_IN_FLIGHT; frame++) {
        for (u32 i = 0; i < layout_count; i++) {
            set_layouts[frame * layout_count + i] = layouts[i];
        }
    }
    
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mgr->pool,
        .descriptorSetCount = RAXTER_MAX_FRAMES_IN_FLIGHT * layout_count,
        .pSetLayouts = set_layouts,
    };
    
    VkDescriptorSet all_sets[RAXTER_MAX_FRAMES_IN_FLIGHT * RAXTER_MAX_DESCRIPTOR_SETS];
    if (vkAllocateDescriptorSets(ctx->device, &alloc_info, all_sets) != VK_SUCCESS) {
        vkDestroyDescriptorPool(ctx->device, mgr->pool, NULL);
        return false;
    }
    
    for (u32 frame = 0; frame < RAXTER_MAX_FRAMES_IN_FLIGHT; frame++) {
        mgr->sets[frame] = all_sets[frame * layout_count];
    }
    
    return true;
}

void vk_descriptor_manager_destroy(vk_context* ctx, vk_descriptor_set_manager* mgr) {
    if (mgr->pool) vkDestroyDescriptorPool(ctx->device, mgr->pool, NULL);
    for (u32 i = 0; i < mgr->layout_count; i++) {
        if (mgr->layouts[i]) vkDestroyDescriptorSetLayout(ctx->device, mgr->layouts[i], NULL);
    }
    memset(mgr, 0, sizeof(vk_descriptor_set_manager));
}

VkDescriptorSet vk_descriptor_manager_get_set(vk_descriptor_set_manager* mgr, u32 frame, u32 layout_idx) {
    if (frame >= RAXTER_MAX_FRAMES_IN_FLIGHT || layout_idx >= mgr->layout_count) return VK_NULL_HANDLE;
    return mgr->sets[frame];
}

void vk_descriptor_manager_update_buffers(vk_context* ctx, vk_descriptor_set_manager* mgr, u32 frame, u32 layout_idx,
                                          u32 binding, VkDescriptorType type, VkBuffer* buffers, u32 count,
                                          VkDeviceSize* offsets, VkDeviceSize* ranges) {
    VkDescriptorSet set = vk_descriptor_manager_get_set(mgr, frame, layout_idx);
    if (set == VK_NULL_HANDLE) return;
    
    VkWriteDescriptorSet writes[16];
    VkDescriptorBufferInfo buf_infos[16];
    
    for (u32 i = 0; i < count; i++) {
        buf_infos[i] = (VkDescriptorBufferInfo){.buffer = buffers[i], .offset = offsets[i], .range = ranges[i]};
        writes[i] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = binding,
            .dstArrayElement = i,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = &buf_infos[i],
        };
    }
    vkUpdateDescriptorSets(ctx->device, count, writes, 0, NULL);
}

void vk_descriptor_manager_update_images(vk_context* ctx, vk_descriptor_set_manager* mgr, u32 frame, u32 layout_idx,
                                         u32 binding, VkDescriptorType type, VkImageView* views, VkSampler* samplers,
                                         VkImageLayout* layouts, u32 count) {
    VkDescriptorSet set = vk_descriptor_manager_get_set(mgr, frame, layout_idx);
    if (set == VK_NULL_HANDLE) return;
    
    VkWriteDescriptorSet writes[16];
    VkDescriptorImageInfo img_infos[16];
    
    for (u32 i = 0; i < count; i++) {
        img_infos[i] = (VkDescriptorImageInfo){
            .sampler = samplers ? samplers[i] : VK_NULL_HANDLE,
            .imageView = views[i],
            .imageLayout = layouts ? layouts[i] : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        writes[i] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = binding,
            .dstArrayElement = i,
            .descriptorCount = 1,
            .descriptorType = type,
            .pImageInfo = &img_infos[i],
        };
    }
    vkUpdateDescriptorSets(ctx->device, count, writes, 0, NULL);
}