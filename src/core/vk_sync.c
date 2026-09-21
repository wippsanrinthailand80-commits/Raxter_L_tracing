#include "vk_sync.h"
#include <string.h>

bool vk_timeline_semaphore_create(vk_context* ctx, vk_timeline_semaphore* sem, u64 initial_value) {
    VkSemaphoreTypeCreateInfo type_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = initial_value,
    };
    VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &type_info,
    };
    VkResult result = vkCreateSemaphore(ctx->device, &info, NULL, &sem->semaphore);
    if (result == VK_SUCCESS) {
        sem->signaled = (initial_value > 0);
        VkFenceCreateInfo fence_info = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCreateFence(ctx->device, &fence_info, NULL, &sem->fence);
    }
    return result == VK_SUCCESS;
}

void vk_timeline_semaphore_destroy(vk_context* ctx, vk_timeline_semaphore* sem) {
    if (sem->fence) vkDestroyFence(ctx->device, sem->fence, NULL);
    if (sem->semaphore) vkDestroySemaphore(ctx->device, sem->semaphore, NULL);
    memset(sem, 0, sizeof(vk_timeline_semaphore));
}

u64 vk_timeline_semaphore_get_value(vk_context* ctx, vk_timeline_semaphore* sem) {
    u64 value = 0;
    vkGetSemaphoreCounterValue(ctx->device, sem->semaphore, &value);
    return value;
}

void vk_timeline_semaphore_signal(vk_context* ctx, vk_timeline_semaphore* sem, u64 value) {
    VkSemaphoreSignalInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
        .semaphore = sem->semaphore,
        .value = value,
    };
    vkSignalSemaphore(ctx->device, &info);
    sem->signaled = true;
}

void vk_timeline_semaphore_wait(vk_context* ctx, vk_timeline_semaphore* sem, u64 value, u64 timeout_ns) {
    VkSemaphoreWaitInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &sem->semaphore,
        .pValues = &value,
    };
    vkWaitSemaphores(ctx->device, &info, timeout_ns);
}

bool vk_event_create(vk_context* ctx, vk_event* event) {
    VkEventCreateInfo info = {.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
    VkResult result = vkCreateEvent(ctx->device, &info, NULL, &event->event);
    event->signaled = false;
    return result == VK_SUCCESS;
}

void vk_event_destroy(vk_context* ctx, vk_event* event) {
    if (event->event) vkDestroyEvent(ctx->device, event->event, NULL);
    memset(event, 0, sizeof(vk_event));
}

void vk_event_set(vk_context* ctx, vk_event* event) {
    vkSetEvent(ctx->device, event->event);
    event->signaled = true;
}

void vk_event_reset(vk_context* ctx, vk_event* event) {
    vkResetEvent(ctx->device, event->event);
    event->signaled = false;
}

VkResult vk_event_wait(vk_context* ctx, vk_event* event, u64 timeout_ns) {
    (void)ctx; (void)event; (void)timeout_ns;
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

void vk_barrier_buffer(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst,
                       VkBuffer buffer, VkAccessFlags src_access, VkAccessFlags dst_access,
                       VkDeviceSize offset, VkDeviceSize size) {
    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = src_access,
        .dstAccessMask = dst_access,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = offset,
        .size = size,
    };
    vkCmdPipelineBarrier(cmd, src, dst, 0, 0, NULL, 1, &barrier, 0, NULL);
}

void vk_barrier_image(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst,
                      VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
                      VkImageAspectFlags aspect, u32 mip_levels, u32 array_layers) {
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = array_layers,
        },
    };
    
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    
    vkCmdPipelineBarrier(cmd, src, dst, 0, 0, NULL, 0, NULL, 1, &barrier);
}

void vk_barrier_global(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst) {
    VkMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
    };
    vkCmdPipelineBarrier(cmd, src, dst, 0, 1, &barrier, 0, NULL, 0, NULL);
}

void vk_memory_barrier(VkCommandBuffer cmd, VkPipelineStageFlags src, VkPipelineStageFlags dst,
                       VkAccessFlags src_access, VkAccessFlags dst_access) {
    VkMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        .srcAccessMask = src_access,
        .dstAccessMask = dst_access,
    };
    vkCmdPipelineBarrier(cmd, src, dst, 0, 1, &barrier, 0, NULL, 0, NULL);
}