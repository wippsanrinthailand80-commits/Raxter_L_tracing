#ifndef RAXTER_VK_CORE_H
#define RAXTER_VK_CORE_H

#include <vulkan/vulkan.h>
#include "raxter_types.h"

#define RAXTER_MAX_FRAMES_IN_FLIGHT 3
#define RAXTER_MAX_QUEUE_FAMILIES 4

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue compute_queue;
    VkQueue present_queue;
    u32 graphics_queue_family;
    u32 compute_queue_family;
    u32 present_queue_family;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
} vk_context;

typedef struct {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat swapchain_format;
    VkColorSpaceKHR color_space;
    VkExtent2D extent;
    u32 image_count;
    VkImage* images;
    VkImageView* image_views;
    VkFramebuffer* framebuffers;
} vk_swapchain;

typedef struct {
    VkCommandPool pool;
    VkCommandBuffer buffers[RAXTER_MAX_FRAMES_IN_FLIGHT];
} vk_command_pool;

typedef struct {
    VkSemaphore image_available[RAXTER_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished[RAXTER_MAX_FRAMES_IN_FLIGHT];
    VkFence in_flight[RAXTER_MAX_FRAMES_IN_FLIGHT];
    u32 current_frame;
} vk_sync_objects;

typedef struct {
    VkDebugUtilsMessengerEXT messenger;
    bool enabled;
} vk_debug;

bool vk_context_create(vk_context* ctx, const char* app_name, bool enable_validation);
void vk_context_destroy(vk_context* ctx);

bool vk_command_pool_create(vk_context* ctx, vk_command_pool* pool, u32 queue_family, VkCommandPoolCreateFlags flags);
void vk_command_pool_destroy(vk_context* ctx, vk_command_pool* pool);

bool vk_sync_objects_create(vk_context* ctx, vk_sync_objects* sync);
void vk_sync_objects_destroy(vk_context* ctx, vk_sync_objects* sync);

bool vk_debug_create(vk_context* ctx, vk_debug* debug);
void vk_debug_destroy(vk_context* ctx, vk_debug* debug);

VkCommandBuffer vk_begin_single_time_commands(vk_context* ctx, VkCommandPool pool);
void vk_end_single_time_commands(vk_context* ctx, VkCommandPool pool, VkCommandBuffer cmd, VkQueue queue);

u32 vk_find_memory_type(vk_context* ctx, u32 type_filter, VkMemoryPropertyFlags properties);

bool vk_swapchain_create(vk_context* ctx, vk_swapchain* swapchain, VkSurfaceKHR surface, u32 width, u32 height);
void vk_swapchain_destroy(vk_context* ctx, vk_swapchain* swapchain);
bool vk_swapchain_recreate(vk_context* ctx, vk_swapchain* swapchain, u32 width, u32 height);

#endif