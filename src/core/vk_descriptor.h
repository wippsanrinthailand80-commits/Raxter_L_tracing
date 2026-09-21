#ifndef RAXTER_VK_DESCRIPTOR_H
#define RAXTER_VK_DESCRIPTOR_H

#include "vk_core.h"
#include "vk_pipeline.h"

bool vk_descriptor_manager_create(vk_context* ctx, vk_descriptor_set_manager* mgr,
                                  VkDescriptorSetLayout* layouts, u32 layout_count,
                                  VkDescriptorPoolSize* pool_sizes, u32 pool_size_count);
void vk_descriptor_manager_destroy(vk_context* ctx, vk_descriptor_set_manager* mgr);

VkDescriptorSet vk_descriptor_manager_get_set(vk_descriptor_set_manager* mgr, u32 frame, u32 layout_idx);

void vk_descriptor_manager_update_buffers(vk_context* ctx, vk_descriptor_set_manager* mgr, u32 frame, u32 layout_idx,
                                          u32 binding, VkDescriptorType type, VkBuffer* buffers, u32 count,
                                          VkDeviceSize* offsets, VkDeviceSize* ranges);

void vk_descriptor_manager_update_images(vk_context* ctx, vk_descriptor_set_manager* mgr, u32 frame, u32 layout_idx,
                                         u32 binding, VkDescriptorType type, VkImageView* views, VkSampler* samplers,
                                         VkImageLayout* layouts, u32 count);

#endif