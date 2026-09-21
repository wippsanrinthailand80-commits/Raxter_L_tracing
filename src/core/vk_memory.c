#include "vk_memory.h"
#include <string.h>

bool vk_buffer_create(vk_context* ctx, vk_buffer* buffer, VkDeviceSize size,
                      VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    memset(buffer, 0, sizeof(vk_buffer));
    buffer->size = size;
    buffer->usage = usage;
    buffer->properties = properties;
    
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    
    VkResult result = vkCreateBuffer(ctx->device, &info, NULL, &buffer->buffer);
    if (result != VK_SUCCESS) return false;
    
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(ctx->device, buffer->buffer, &mem_req);
    
    u32 mem_type = vk_find_memory_type(ctx, mem_req.memoryTypeBits, properties);
    if (mem_type == UINT32_MAX) {
        vkDestroyBuffer(ctx->device, buffer->buffer, NULL);
        return false;
    }
    
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_req.size,
        .memoryTypeIndex = mem_type,
    };
    
    result = vkAllocateMemory(ctx->device, &alloc_info, NULL, &buffer->memory);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(ctx->device, buffer->buffer, NULL);
        return false;
    }
    
    vkBindBufferMemory(ctx->device, buffer->buffer, buffer->memory, 0);
    return true;
}

void vk_buffer_destroy(vk_context* ctx, vk_buffer* buffer) {
    if (buffer->mapped) vkUnmapMemory(ctx->device, buffer->memory);
    if (buffer->memory) vkFreeMemory(ctx->device, buffer->memory, NULL);
    if (buffer->buffer) vkDestroyBuffer(ctx->device, buffer->buffer, NULL);
    memset(buffer, 0, sizeof(vk_buffer));
}

void* vk_buffer_map(vk_context* ctx, vk_buffer* buffer, VkDeviceSize offset, VkDeviceSize size) {
    if (buffer->mapped) return buffer->mapped;
    VkResult result = vkMapMemory(ctx->device, buffer->memory, offset, size, 0, &buffer->mapped);
    return result == VK_SUCCESS ? buffer->mapped : NULL;
}

void vk_buffer_unmap(vk_context* ctx, vk_buffer* buffer) {
    if (buffer->mapped) {
        vkUnmapMemory(ctx->device, buffer->memory);
        buffer->mapped = NULL;
    }
}

void vk_buffer_copy(vk_context* ctx, VkCommandBuffer cmd, vk_buffer* src, vk_buffer* dst, VkDeviceSize size) {
    VkBufferCopy region = {.srcOffset = 0, .dstOffset = 0, .size = size};
    vkCmdCopyBuffer(cmd, src->buffer, dst->buffer, 1, &region);
}

bool vk_image_create(vk_context* ctx, vk_image* image, VkImageType type, VkFormat format,
                     VkExtent3D extent, u32 mip_levels, u32 array_layers, VkSampleCountFlagBits samples,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
    memset(image, 0, sizeof(vk_image));
    image->format = format;
    image->extent = extent;
    image->mip_levels = mip_levels;
    image->array_layers = array_layers;
    image->usage = usage;
    image->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    VkImageCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = type,
        .format = format,
        .extent = extent,
        .mipLevels = mip_levels,
        .arrayLayers = array_layers,
        .samples = samples,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    
    VkResult result = vkCreateImage(ctx->device, &info, NULL, &image->image);
    if (result != VK_SUCCESS) return false;
    
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(ctx->device, image->image, &mem_req);
    
    u32 mem_type = vk_find_memory_type(ctx, mem_req.memoryTypeBits, properties);
    if (mem_type == UINT32_MAX) {
        vkDestroyImage(ctx->device, image->image, NULL);
        return false;
    }
    
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_req.size,
        .memoryTypeIndex = mem_type,
    };
    
    result = vkAllocateMemory(ctx->device, &alloc_info, NULL, &image->memory);
    if (result != VK_SUCCESS) {
        vkDestroyImage(ctx->device, image->image, NULL);
        return false;
    }
    
    vkBindImageMemory(ctx->device, image->image, image->memory, 0);
    return true;
}

void vk_image_destroy(vk_context* ctx, vk_image* image) {
    if (image->view) vkDestroyImageView(ctx->device, image->view, NULL);
    if (image->memory) vkFreeMemory(ctx->device, image->memory, NULL);
    if (image->image) vkDestroyImage(ctx->device, image->image, NULL);
    memset(image, 0, sizeof(vk_image));
}

bool vk_image_view_create(vk_context* ctx, vk_image* image, VkImageViewType view_type, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image->image,
        .viewType = view_type,
        .format = image->format,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = image->mip_levels,
            .baseArrayLayer = 0,
            .layerCount = image->array_layers,
        },
    };
    return vkCreateImageView(ctx->device, &info, NULL, &image->view) == VK_SUCCESS;
}

void vk_image_transition_layout(vk_context* ctx, VkCommandBuffer cmd, vk_image* image,
                                VkImageLayout old_layout, VkImageLayout new_layout,
                                VkImageAspectFlags aspect, u32 mip_levels, u32 array_layers) {
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->image,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = array_layers,
        },
    };
    
    VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dst_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    
    vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
    image->layout = new_layout;
}

void vk_image_copy_buffer_to_image(vk_context* ctx, VkCommandBuffer cmd, vk_buffer* buffer, vk_image* image, VkExtent3D extent) {
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = extent,
    };
    vkCmdCopyBufferToImage(cmd, buffer->buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

bool vk_sampler_create(vk_context* ctx, VkSampler* sampler, VkFilter mag_filter, VkFilter min_filter,
                       VkSamplerAddressMode address_mode, f32 max_anisotropy, bool compare_op) {
    VkSamplerCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = mag_filter,
        .minFilter = min_filter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = address_mode,
        .addressModeV = address_mode,
        .addressModeW = address_mode,
        .mipLodBias = 0.0f,
        .anisotropyEnable = max_anisotropy > 1.0f ? VK_TRUE : VK_FALSE,
        .maxAnisotropy = max_anisotropy,
        .compareEnable = compare_op ? VK_TRUE : VK_FALSE,
        .compareOp = compare_op ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    return vkCreateSampler(ctx->device, &info, NULL, sampler) == VK_SUCCESS;
}

void vk_sampler_destroy(vk_context* ctx, VkSampler sampler) {
    if (sampler) vkDestroySampler(ctx->device, sampler, NULL);
}