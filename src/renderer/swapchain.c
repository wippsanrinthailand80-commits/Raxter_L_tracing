#include "core/vk_swapchain.h"
#include "core/vk_core.h"
#include "core/vk_memory.h"
#include <stdlib.h>
#include <string.h>

bool vk_swapchain_create(vk_context* ctx, vk_swapchain* swapchain, VkSurfaceKHR surface, u32 width, u32 height) {
    memset(swapchain, 0, sizeof(vk_swapchain));
    swapchain->surface = surface;
    
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, surface, &caps);
    
    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, surface, &format_count, NULL);
    VkSurfaceFormatKHR* formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, surface, &format_count, formats);
    
    swapchain->swapchain_format = formats[0].format;
    swapchain->color_space = formats[0].colorSpace;
    for (u32 i = 0; i < format_count; i++) {
        if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB || 
            formats[i].format == VK_FORMAT_B8G8R8A8_SRGB) {
            swapchain->swapchain_format = formats[i].format;
            swapchain->color_space = formats[i].colorSpace;
            break;
        }
    }
    free(formats);
    
    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, surface, &present_mode_count, NULL);
    VkPresentModeKHR* present_modes = malloc(present_mode_count * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, surface, &present_mode_count, present_modes);
    
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < present_mode_count; i++) {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
    free(present_modes);
    
    swapchain->extent.width = RAXTER_CLAMP(width, caps.minImageExtent.width, caps.maxImageExtent.width);
    swapchain->extent.height = RAXTER_CLAMP(height, caps.minImageExtent.height, caps.maxImageExtent.height);
    
    swapchain->image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && swapchain->image_count > caps.maxImageCount) {
        swapchain->image_count = caps.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = swapchain->image_count,
        .imageFormat = swapchain->swapchain_format,
        .imageColorSpace = swapchain->color_space,
        .imageExtent = swapchain->extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    
    VkResult result = vkCreateSwapchainKHR(ctx->device, &create_info, NULL, &swapchain->swapchain);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    vkGetSwapchainImagesKHR(ctx->device, swapchain->swapchain, &swapchain->image_count, NULL);
    swapchain->images = malloc(swapchain->image_count * sizeof(VkImage));
    swapchain->image_views = malloc(swapchain->image_count * sizeof(VkImageView));
    swapchain->framebuffers = malloc(swapchain->image_count * sizeof(VkFramebuffer));
    vkGetSwapchainImagesKHR(ctx->device, swapchain->swapchain, &swapchain->image_count, swapchain->images);
    
    for (u32 i = 0; i < swapchain->image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->swapchain_format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        vkCreateImageView(ctx->device, &view_info, NULL, &swapchain->image_views[i]);
    }
    
    return true;
}

void vk_swapchain_destroy(vk_context* ctx, vk_swapchain* swapchain) {
    if (swapchain->framebuffers) {
        for (u32 i = 0; i < swapchain->image_count; i++) {
            if (swapchain->framebuffers[i]) vkDestroyFramebuffer(ctx->device, swapchain->framebuffers[i], NULL);
        }
        free(swapchain->framebuffers);
    }
    if (swapchain->image_views) {
        for (u32 i = 0; i < swapchain->image_count; i++) {
            if (swapchain->image_views[i]) vkDestroyImageView(ctx->device, swapchain->image_views[i], NULL);
        }
        free(swapchain->image_views);
    }
    if (swapchain->images) free(swapchain->images);
    if (swapchain->swapchain) vkDestroySwapchainKHR(ctx->device, swapchain->swapchain, NULL);
    memset(swapchain, 0, sizeof(vk_swapchain));
}

bool vk_swapchain_recreate(vk_context* ctx, vk_swapchain* swapchain, u32 width, u32 height) {
    vkDeviceWaitIdle(ctx->device);
    VkSwapchainKHR old_swapchain = swapchain->swapchain;
    
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, swapchain->surface, &caps);
    
    swapchain->extent.width = RAXTER_CLAMP(width, caps.minImageExtent.width, caps.maxImageExtent.width);
    swapchain->extent.height = RAXTER_CLAMP(height, caps.minImageExtent.height, caps.maxImageExtent.height);
    
    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = swapchain->surface,
        .minImageCount = swapchain->image_count,
        .imageFormat = swapchain->swapchain_format,
        .imageColorSpace = swapchain->color_space,
        .imageExtent = swapchain->extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    };
    
    VkSwapchainKHR new_swapchain;
    VkResult result = vkCreateSwapchainKHR(ctx->device, &create_info, NULL, &new_swapchain);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    for (u32 i = 0; i < swapchain->image_count; i++) {
        if (swapchain->framebuffers[i]) vkDestroyFramebuffer(ctx->device, swapchain->framebuffers[i], NULL);
        if (swapchain->image_views[i]) vkDestroyImageView(ctx->device, swapchain->image_views[i], NULL);
    }
    
    swapchain->swapchain = new_swapchain;
    vkGetSwapchainImagesKHR(ctx->device, swapchain->swapchain, &swapchain->image_count, NULL);
    free(swapchain->images);
    free(swapchain->image_views);
    swapchain->images = malloc(swapchain->image_count * sizeof(VkImage));
    swapchain->image_views = malloc(swapchain->image_count * sizeof(VkImageView));
    swapchain->framebuffers = malloc(swapchain->image_count * sizeof(VkFramebuffer));
    vkGetSwapchainImagesKHR(ctx->device, swapchain->swapchain, &swapchain->image_count, swapchain->images);
    
    for (u32 i = 0; i < swapchain->image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->swapchain_format,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        vkCreateImageView(ctx->device, &view_info, NULL, &swapchain->image_views[i]);
    }
    
    vkDestroySwapchainKHR(ctx->device, old_swapchain, NULL);
    return true;
}