#ifndef RAXTER_VK_SYNC_H
#define RAXTER_VK_SYNC_H

#include "vk_core.h"

typedef struct {
    VkFence fence;
    VkSemaphore semaphore;
    bool signaled;
} vk_timeline_semaphore;

typedef struct {
    VkEvent event;
    bool signaled;
} vk_event;

bool vk_timeline_semaphore_create(vk_context* ctx, vk_timeline_semaphore* sem, u64 initial_value);
void vk_timeline_semaphore_destroy(vk_context* ctx, vk_timeline_semaphore* sem);
u64 vk_timeline_semaphore_get_value(vk_context* ctx, vk_timeline_semaphore* sem);
void vk_timeline_semaphore_signal(vk_context* ctx, vk_timeline_semaphore* sem, u64 value);
void vk_timeline_semaphore_wait(vk_context* ctx, vk_timeline_semaphore* sem, u64 value, u64 timeout_ns);

bool vk_event_create(vk_context* ctx, vk_event* event);
void vk_event_destroy(vk_context* ctx, vk_event* event);
void vk_event_set(vk_context* ctx, vk_event* event);
void vk_event_reset(vk_context* ctx, vk_event* event);
VkResult vk_event_wait(vk_context* ctx, vk_event* event, u64 timeout_ns);

void vk_barrier_buffer(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst,
                       VkBuffer buffer, VkAccessFlags src_access, VkAccessFlags dst_access,
                       VkDeviceSize offset, VkDeviceSize size);

void vk_barrier_image(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst,
                      VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
                      VkImageAspectFlags aspect, u32 mip_levels, u32 array_layers);

void vk_barrier_global(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst);

void vk_memory_barrier(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst,
                       VkAccessFlags src_access, VkAccessFlags dst_access);

#endif