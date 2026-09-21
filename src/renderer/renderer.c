#include "renderer.h"
#include "core/vk_core.h"
#include "core/vk_swapchain.h"
#include "core/vk_memory.h"
#include "core/vk_pipeline.h"
#include "core/vk_descriptor.h"
#include "core/vk_sync.h"
#include "core/vk_debug.h"
#include "compute/wave_optics.h"
#include "compute/interference.h"
#include "compute/diffraction.h"
#include "compute/polarization.h"
#include "compute/light_propagation.h"
#include "compute/quantum_coherence.h"
#include "compute/oam_beams.h"
#include "compute/spatiotemporal_packets.h"
#include "compute/four_wave_mixing.h"
#include "compute/casimir_effect.h"
#include "compute/ghost_imaging.h"
#include "platform/window.h"
#include "core/gpu_detect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void transition_swapchain_image(vk_context* ctx, VkCommandBuffer cmd, VkImage image, 
                                       VkImageLayout old_layout, VkImageLayout new_layout) {
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = 0;
    } else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = 0;
    }
    
    VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    
    vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
}

static void copy_image_to_swapchain(vk_context* ctx, VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D extent) {
    VkImageCopy region = {
        .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .srcOffset = {0, 0, 0},
        .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .dstOffset = {0, 0, 0},
        .extent = {extent.width, extent.height, 1},
    };
    vkCmdCopyImage(cmd, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

bool raxter_renderer_create(raxter_renderer* renderer, const char* app_name, u32 width, u32 height, u32 max_bounces, bool validation) {
    memset(renderer, 0, sizeof(raxter_renderer));
    renderer->width = width;
    renderer->height = height;
    renderer->max_bounces = RAXTER_CLAMP(max_bounces, 1, 8);
    renderer->validation_enabled = validation;
    renderer->current_mode = RAXTER_RENDER_MODE_WAVE_OPTICS;
    
    if (!vk_context_create(&renderer->vk, app_name, validation)) {
        fprintf(stderr, "Failed to create Vulkan context\n");
        return false;
    }
    
    VkSurfaceKHR surface = platform_create_surface(renderer->vk.instance, width, height);
    if (surface == VK_NULL_HANDLE) {
        fprintf(stderr, "Failed to create surface\n");
        vk_context_destroy(&renderer->vk);
        return false;
    }
    
    if (!vk_swapchain_create(&renderer->vk, &renderer->swapchain, surface, width, height)) {
        fprintf(stderr, "Failed to create swapchain\n");
        vk_context_destroy(&renderer->vk);
        return false;
    }
    
    if (!vk_command_pool_create(&renderer->vk, &renderer->graphics_pool, renderer->vk.graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)) {
        fprintf(stderr, "Failed to create graphics command pool\n");
        vk_swapchain_destroy(&renderer->vk, &renderer->swapchain);
        vk_context_destroy(&renderer->vk);
        return false;
    }
    
    if (!vk_command_pool_create(&renderer->vk, &renderer->compute_pool, renderer->vk.compute_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)) {
        fprintf(stderr, "Failed to create compute command pool\n");
        vk_command_pool_destroy(&renderer->vk, &renderer->graphics_pool);
        vk_swapchain_destroy(&renderer->vk, &renderer->swapchain);
        vk_context_destroy(&renderer->vk);
        return false;
    }
    
    if (!vk_sync_objects_create(&renderer->vk, &renderer->sync)) {
        fprintf(stderr, "Failed to create sync objects\n");
        vk_command_pool_destroy(&renderer->vk, &renderer->compute_pool);
        vk_command_pool_destroy(&renderer->vk, &renderer->graphics_pool);
        vk_swapchain_destroy(&renderer->vk, &renderer->swapchain);
        vk_context_destroy(&renderer->vk);
        return false;
    }
    
    if (validation && !vk_debug_create(&renderer->vk, &renderer->debug)) {
        fprintf(stderr, "Warning: Failed to create debug messenger\n");
    }
    
    renderer->auto_quality = true;
    raxter_renderer_auto_quality(renderer);
    
    if (!wave_optics_create(&renderer->vk, &renderer->wave_optics, width, height, renderer->quality.max_bounces)) {
        fprintf(stderr, "Failed to create wave optics renderer\n");
        return false;
    }
    
    if (!interference_create(&renderer->vk, &renderer->interference, width, height)) {
        fprintf(stderr, "Failed to create interference renderer\n");
        return false;
    }
    
    if (!diffraction_create(&renderer->vk, &renderer->diffraction, width, height)) {
        fprintf(stderr, "Failed to create diffraction renderer\n");
        return false;
    }
    
    if (!polarization_create(&renderer->vk, &renderer->polarization, width, height)) {
        fprintf(stderr, "Failed to create polarization renderer\n");
        return false;
    }
    
    if (!light_propagation_create(&renderer->vk, &renderer->light_propagation, renderer->quality.volume_dim)) {
        fprintf(stderr, "Failed to create light propagation renderer\n");
        return false;
    }
    
    if (!quantum_coherence_create(&renderer->vk, &renderer->quantum_coherence, width, height)) {
        fprintf(stderr, "Failed to create quantum coherence renderer\n");
        return false;
    }
    
    if (!oam_beams_create(&renderer->vk, &renderer->oam_beams, width, height)) {
        fprintf(stderr, "Failed to create OAM beams renderer\n");
        return false;
    }
    
    if (!spatiotemporal_packets_create(&renderer->vk, &renderer->spatiotemporal_packets, renderer->quality.volume_dim)) {
        fprintf(stderr, "Failed to create spatiotemporal packets renderer\n");
        return false;
    }
    
    if (!four_wave_mixing_create(&renderer->vk, &renderer->four_wave_mixing, width, height)) {
        fprintf(stderr, "Failed to create four wave mixing renderer\n");
        return false;
    }
    
    if (!casimir_effect_create(&renderer->vk, &renderer->casimir_effect, width, height)) {
        fprintf(stderr, "Failed to create Casimir effect renderer\n");
        return false;
    }
    
    if (!ghost_imaging_create(&renderer->vk, &renderer->ghost_imaging, width, height)) {
        fprintf(stderr, "Failed to create ghost imaging renderer\n");
        return false;
    }
    
    return true;
}

void raxter_renderer_destroy(raxter_renderer* renderer) {
    vkDeviceWaitIdle(renderer->vk.device);
    
    ghost_imaging_destroy(&renderer->vk, &renderer->ghost_imaging);
    casimir_effect_destroy(&renderer->vk, &renderer->casimir_effect);
    four_wave_mixing_destroy(&renderer->vk, &renderer->four_wave_mixing);
    spatiotemporal_packets_destroy(&renderer->vk, &renderer->spatiotemporal_packets);
    oam_beams_destroy(&renderer->vk, &renderer->oam_beams);
    quantum_coherence_destroy(&renderer->vk, &renderer->quantum_coherence);
    light_propagation_destroy(&renderer->vk, &renderer->light_propagation);
    polarization_destroy(&renderer->vk, &renderer->polarization);
    diffraction_destroy(&renderer->vk, &renderer->diffraction);
    interference_destroy(&renderer->vk, &renderer->interference);
    wave_optics_destroy(&renderer->vk, &renderer->wave_optics);
    
    vk_debug_destroy(&renderer->vk, &renderer->debug);
    vk_sync_objects_destroy(&renderer->vk, &renderer->sync);
    vk_command_pool_destroy(&renderer->vk, &renderer->compute_pool);
    vk_command_pool_destroy(&renderer->vk, &renderer->graphics_pool);
    vk_swapchain_destroy(&renderer->vk, &renderer->swapchain);
    vk_context_destroy(&renderer->vk);
    
    memset(renderer, 0, sizeof(raxter_renderer));
}

bool raxter_renderer_begin_frame(raxter_renderer* renderer) {
    vkWaitForFences(renderer->vk.device, 1, &renderer->sync.in_flight[renderer->sync.current_frame], VK_TRUE, UINT64_MAX);
    vkResetFences(renderer->vk.device, 1, &renderer->sync.in_flight[renderer->sync.current_frame]);
    
    u32 image_index;
    VkResult result = vkAcquireNextImageKHR(renderer->vk.device, renderer->swapchain.swapchain, UINT64_MAX,
                                            renderer->sync.image_available[renderer->sync.current_frame],
                                            VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        raxter_renderer_resize(renderer, renderer->width, renderer->height);
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "Failed to acquire swapchain image: %d\n", result);
        return false;
    }
    
    VkCommandBuffer cmd = renderer->graphics_pool.buffers[renderer->sync.current_frame];
    vkResetCommandBuffer(cmd, 0);
    
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(cmd, &begin_info);
    
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = renderer->swapchain.images[image_index],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier);
    
    return true;
}

void raxter_renderer_end_frame(raxter_renderer* renderer) {
    VkCommandBuffer cmd = renderer->graphics_pool.buffers[renderer->sync.current_frame];
    vkEndCommandBuffer(cmd);
    
    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->sync.image_available[renderer->sync.current_frame],
        .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderer->sync.render_finished[renderer->sync.current_frame],
    };
    
    vkQueueSubmit(renderer->vk.graphics_queue, 1, &submit, renderer->sync.in_flight[renderer->sync.current_frame]);
    
    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->sync.render_finished[renderer->sync.current_frame],
        .swapchainCount = 1,
        .pSwapchains = &renderer->swapchain.swapchain,
        .pImageIndices = &(u32){0},
    };
    
    vkQueuePresentKHR(renderer->vk.present_queue, &present);
    
    renderer->sync.current_frame = (renderer->sync.current_frame + 1) % RAXTER_MAX_FRAMES_IN_FLIGHT;
    renderer->frame_count++;
}

void raxter_renderer_set_mode(raxter_renderer* renderer, raxter_render_mode mode) {
    renderer->current_mode = mode;
}

void raxter_renderer_resize(raxter_renderer* renderer, u32 width, u32 height) {
    vkDeviceWaitIdle(renderer->vk.device);
    
    renderer->width = width;
    renderer->height = height;
    
    vk_swapchain_destroy(&renderer->vk, &renderer->swapchain);
    VkSurfaceKHR surface = platform_create_surface(renderer->vk.instance, width, height);
    vk_swapchain_create(&renderer->vk, &renderer->swapchain, surface, width, height);
    
    wave_optics_resize(&renderer->wave_optics, width, height);
    interference_destroy(&renderer->vk, &renderer->interference);
    interference_create(&renderer->vk, &renderer->interference, width, height);
    diffraction_destroy(&renderer->vk, &renderer->diffraction);
    diffraction_create(&renderer->vk, &renderer->diffraction, width, height);
    polarization_destroy(&renderer->vk, &renderer->polarization);
    polarization_create(&renderer->vk, &renderer->polarization, width, height);
}

void raxter_renderer_update(raxter_renderer* renderer, f32 delta_time) {
    renderer->frame_time += delta_time;
    
    VkCommandBuffer compute_cmd = renderer->compute_pool.buffers[renderer->sync.current_frame];
    vkResetCommandBuffer(compute_cmd, 0);
    
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(compute_cmd, &begin_info);
    
    switch (renderer->current_mode) {
        case RAXTER_RENDER_MODE_WAVE_OPTICS:
            wave_optics_render(&renderer->vk, &renderer->wave_optics, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_INTERFERENCE:
            interference_render(&renderer->vk, &renderer->interference, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_DIFFRACTION:
            diffraction_render(&renderer->vk, &renderer->diffraction, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_POLARIZATION:
            polarization_render(&renderer->vk, &renderer->polarization, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_LIGHT_PROPAGATION:
            light_propagation_render(&renderer->vk, &renderer->light_propagation, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_QUANTUM_COHERENCE:
            quantum_coherence_render(&renderer->vk, &renderer->quantum_coherence, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_OAM_BEAMS:
            oam_beams_render(&renderer->vk, &renderer->oam_beams, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_SPATIOTEMPORAL_PACKETS:
            spatiotemporal_packets_render(&renderer->vk, &renderer->spatiotemporal_packets, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_FOUR_WAVE_MIXING:
            four_wave_mixing_render(&renderer->vk, &renderer->four_wave_mixing, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_CASIMIR_EFFECT:
            casimir_effect_render(&renderer->vk, &renderer->casimir_effect, compute_cmd, renderer->sync.current_frame);
            break;
        case RAXTER_RENDER_MODE_GHOST_IMAGING:
            ghost_imaging_render(&renderer->vk, &renderer->ghost_imaging, compute_cmd, renderer->sync.current_frame);
            break;
    }
    
    vkEndCommandBuffer(compute_cmd);
    
    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &compute_cmd,
    };
    vkQueueSubmit(renderer->vk.compute_queue, 1, &submit, VK_NULL_HANDLE);
}

void raxter_renderer_auto_quality(raxter_renderer* renderer) {
    renderer->gpu_tier = gpu_detect_tier(&renderer->vk);
    renderer->quality_preset = quality_preset_for_tier(renderer->gpu_tier);
    quality_settings_apply(renderer->quality_preset, &renderer->quality);
    renderer->max_bounces = renderer->quality.max_bounces;
    printf("Auto quality: %s (GPU tier: %s)\n", 
           quality_preset_name(renderer->quality_preset), 
           gpu_tier_name(renderer->gpu_tier));
}

void raxter_renderer_set_quality(raxter_renderer* renderer, quality_preset preset) {
    renderer->auto_quality = false;
    renderer->quality_preset = preset;
    quality_settings_apply(preset, &renderer->quality);
    renderer->max_bounces = renderer->quality.max_bounces;
    printf("Quality set to: %s\n", quality_preset_name(preset));
}

void raxter_renderer_apply_quality(raxter_renderer* renderer) {
    renderer->max_bounces = renderer->quality.max_bounces;
    printf("Quality applied: bounces=%u, spectral=%u, volume=%u, oam_charge=%u, volume_prop=%d, fwm=%d, casimir=%d, ghost=%d, scale=%.2f\n",
           renderer->quality.max_bounces,
           renderer->quality.spectral_samples,
           renderer->quality.volume_dim,
           renderer->quality.max_oam_charge,
           renderer->quality.enable_volume_propagation,
           renderer->quality.enable_four_wave_mixing,
           renderer->quality.enable_casimir,
           renderer->quality.enable_ghost_imaging,
           renderer->quality.render_scale);
}