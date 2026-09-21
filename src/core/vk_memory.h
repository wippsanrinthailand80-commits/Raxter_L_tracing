#ifndef RAXTER_VK_MEMORY_H
#define RAXTER_VK_MEMORY_H

#include "vk_core.h"

typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
    void* mapped;
} vk_buffer;

typedef struct {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkFormat format;
    VkExtent3D extent;
    u32 mip_levels;
    u32 array_layers;
    VkImageUsageFlags usage;
    VkImageLayout layout;
} vk_image;

bool vk_buffer_create(vk_context* ctx, vk_buffer* buffer, VkDeviceSize size, 
                      VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
void vk_buffer_destroy(vk_context* ctx, vk_buffer* buffer);
void* vk_buffer_map(vk_context* ctx, vk_buffer* buffer, VkDeviceSize offset, VkDeviceSize size);
void vk_buffer_unmap(vk_context* ctx, vk_buffer* buffer);
void vk_buffer_copy(vk_context* ctx, VkCommandBuffer cmd, vk_buffer* src, vk_buffer* dst, VkDeviceSize size);

bool vk_image_create(vk_context* ctx, vk_image* image, VkImageType type, VkFormat format,
                     VkExtent3D extent, u32 mip_levels, u32 array_layers, VkSampleCountFlagBits samples,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
void vk_image_destroy(vk_context* ctx, vk_image* image);
bool vk_image_view_create(vk_context* ctx, vk_image* image, VkImageViewType view_type, VkImageAspectFlags aspect);
void vk_image_transition_layout(vk_context* ctx, VkCommandBuffer cmd, vk_image* image,
                                VkImageLayout old_layout, VkImageLayout new_layout,
                                VkImageAspectFlags aspect, u32 mip_levels, u32 array_layers);
void vk_image_copy_buffer_to_image(vk_context* ctx, VkCommandBuffer cmd, vk_buffer* buffer, vk_image* image, VkExtent3D extent);

bool vk_sampler_create(vk_context* ctx, VkSampler* sampler, VkFilter mag_filter, VkFilter min_filter,
                       VkSamplerAddressMode address_mode, f32 max_anisotropy, bool compare_op);
void vk_sampler_destroy(vk_context* ctx, VkSampler sampler);

#endif