#include "vk_core.h"
#include "vk_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* REQUIRED_INSTANCE_EXTENSIONS[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
};

static const u32 REQUIRED_INSTANCE_EXTENSION_COUNT = 4;

static const char* REQUIRED_DEVICE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
};

static const u32 REQUIRED_DEVICE_EXTENSION_COUNT = 2;

static const char* VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation",
};

static const u32 VALIDATION_LAYER_COUNT = 1;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    
    (void)type; (void)user_data;
    
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "[Vulkan Validation] %s\n", callback_data->pMessage);
    }
    return VK_FALSE;
}

static bool check_instance_extensions() {
    u32 count;
    vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
    VkExtensionProperties* props = malloc(count * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &count, props);
    
    for (u32 i = 0; i < REQUIRED_INSTANCE_EXTENSION_COUNT; i++) {
        bool found = false;
        for (u32 j = 0; j < count; j++) {
            if (strcmp(REQUIRED_INSTANCE_EXTENSIONS[i], props[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Missing instance extension: %s\n", REQUIRED_INSTANCE_EXTENSIONS[i]);
            free(props);
            return false;
        }
    }
    free(props);
    return true;
}

static bool check_device_extensions(VkPhysicalDevice device) {
    u32 count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
    VkExtensionProperties* props = malloc(count * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &count, props);
    
    for (u32 i = 0; i < REQUIRED_DEVICE_EXTENSION_COUNT; i++) {
        bool found = false;
        for (u32 j = 0; j < count; j++) {
            if (strcmp(REQUIRED_DEVICE_EXTENSIONS[i], props[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Missing device extension: %s\n", REQUIRED_DEVICE_EXTENSIONS[i]);
            free(props);
            return false;
        }
    }
    free(props);
    return true;
}

static bool check_validation_layers() {
    u32 count;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    VkLayerProperties* props = malloc(count * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&count, props);
    
    for (u32 i = 0; i < VALIDATION_LAYER_COUNT; i++) {
        bool found = false;
        for (u32 j = 0; j < count; j++) {
            if (strcmp(VALIDATION_LAYERS[i], props[j].layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Missing validation layer: %s\n", VALIDATION_LAYERS[i]);
            free(props);
            return false;
        }
    }
    free(props);
    return true;
}

static int rate_device(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    
    int score = 0;
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
    else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 500;
    
    score += props.limits.maxImageDimension2D;
    return score;
}

static bool find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface, 
                                u32* graphics_family, u32* compute_family, u32* present_family) {
    u32 count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
    VkQueueFamilyProperties* props = malloc(count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, props);
    
    *graphics_family = UINT32_MAX;
    *compute_family = UINT32_MAX;
    *present_family = UINT32_MAX;
    
    for (u32 i = 0; i < count; i++) {
        if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            if (*graphics_family == UINT32_MAX) *graphics_family = i;
        }
        if (props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            if (*compute_family == UINT32_MAX || !(props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                *compute_family = i;
            }
        }
        if (surface != VK_NULL_HANDLE) {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
            if (present_support && *present_family == UINT32_MAX) *present_family = i;
        }
    }
    
    free(props);
    return *graphics_family != UINT32_MAX && *compute_family != UINT32_MAX;
}

bool vk_context_create(vk_context* ctx, const char* app_name, bool enable_validation) {
    memset(ctx, 0, sizeof(vk_context));
    
    if (!check_instance_extensions()) return false;
    
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app_name,
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "Raxter L Tracing",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_1,
    };
    
    const char** extensions = REQUIRED_INSTANCE_EXTENSIONS;
    u32 extension_count = REQUIRED_INSTANCE_EXTENSION_COUNT;
    
    const char** layers = NULL;
    u32 layer_count = 0;
    
    if (enable_validation) {
        if (!check_validation_layers()) return false;
        layers = VALIDATION_LAYERS;
        layer_count = VALIDATION_LAYER_COUNT;
        
        static const char* debug_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        const char** ext_array = malloc((extension_count + 1) * sizeof(char*));
        memcpy(ext_array, extensions, extension_count * sizeof(char*));
        ext_array[extension_count++] = debug_ext;
        extensions = ext_array;
    }
    
    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = layer_count,
        .ppEnabledLayerNames = layers,
    };
    
    VkResult result = vkCreateInstance(&create_info, NULL, &ctx->instance);
    if (extensions != REQUIRED_INSTANCE_EXTENSIONS) {
        free((void*)extensions);
    }
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance: %d\n", result);
        return false;
    }
    
    u32 device_count;
    vkEnumeratePhysicalDevices(ctx->instance, &device_count, NULL);
    if (device_count == 0) {
        fprintf(stderr, "No Vulkan devices found\n");
        return false;
    }
    
    VkPhysicalDevice* devices = malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(ctx->instance, &device_count, devices);
    
    int best_score = -1;
    for (u32 i = 0; i < device_count; i++) {
        if (!check_device_extensions(devices[i])) continue;
        
        u32 gfx, comp, pres;
        if (!find_queue_families(devices[i], VK_NULL_HANDLE, &gfx, &comp, &pres)) continue;
        
        int score = rate_device(devices[i]);
        if (score > best_score) {
            best_score = score;
            ctx->physical_device = devices[i];
            ctx->graphics_queue_family = gfx;
            ctx->compute_queue_family = comp;
            ctx->present_queue_family = pres;
        }
    }
    
    free(devices);
    
    if (ctx->physical_device == VK_NULL_HANDLE) {
        fprintf(stderr, "No suitable GPU found\n");
        return false;
    }
    
    vkGetPhysicalDeviceProperties(ctx->physical_device, &ctx->properties);
    vkGetPhysicalDeviceFeatures(ctx->physical_device, &ctx->features);
    vkGetPhysicalDeviceMemoryProperties(ctx->physical_device, &ctx->memory_properties);
    
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_infos[3] = {0};
    u32 queue_count = 0;
    
    u32 unique_families[3] = {
        ctx->graphics_queue_family,
        ctx->compute_queue_family,
        ctx->present_queue_family,
    };
    
    for (u32 i = 0; i < 3; i++) {
        bool unique = true;
        for (u32 j = 0; j < i; j++) {
            if (unique_families[i] == unique_families[j]) {
                unique = false;
                break;
            }
        }
        if (unique) {
            queue_infos[queue_count] = (VkDeviceQueueCreateInfo){
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = unique_families[i],
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            };
            queue_count++;
        }
    }
    
    VkPhysicalDeviceFeatures device_features = {0};
    device_features.shaderInt64 = VK_TRUE;
    device_features.shaderFloat64 = VK_TRUE;
    device_features.multiDrawIndirect = VK_TRUE;
    device_features.drawIndirectFirstInstance = VK_TRUE;
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.sampleRateShading = VK_TRUE;
    
    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queue_count,
        .pQueueCreateInfos = queue_infos,
        .enabledExtensionCount = REQUIRED_DEVICE_EXTENSION_COUNT,
        .ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS,
        .pEnabledFeatures = &device_features,
    };
    
    result = vkCreateDevice(ctx->physical_device, &device_info, NULL, &ctx->device);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device: %d\n", result);
        return false;
    }
    
    vkGetDeviceQueue(ctx->device, ctx->graphics_queue_family, 0, &ctx->graphics_queue);
    vkGetDeviceQueue(ctx->device, ctx->compute_queue_family, 0, &ctx->compute_queue);
    vkGetDeviceQueue(ctx->device, ctx->present_queue_family, 0, &ctx->present_queue);
    
    return true;
}

void vk_context_destroy(vk_context* ctx) {
    if (ctx->device) vkDestroyDevice(ctx->device, NULL);
    if (ctx->instance) vkDestroyInstance(ctx->instance, NULL);
    memset(ctx, 0, sizeof(vk_context));
}

bool vk_command_pool_create(vk_context* ctx, vk_command_pool* pool, u32 queue_family, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queue_family,
    };
    VkResult result = vkCreateCommandPool(ctx->device, &info, NULL, &pool->pool);
    if (result != VK_SUCCESS) return false;
    
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool->pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = RAXTER_MAX_FRAMES_IN_FLIGHT,
    };
    result = vkAllocateCommandBuffers(ctx->device, &alloc_info, pool->buffers);
    return result == VK_SUCCESS;
}

void vk_command_pool_destroy(vk_context* ctx, vk_command_pool* pool) {
    if (pool->pool) vkDestroyCommandPool(ctx->device, pool->pool, NULL);
    memset(pool, 0, sizeof(vk_command_pool));
}

bool vk_sync_objects_create(vk_context* ctx, vk_sync_objects* sync) {
    VkSemaphoreCreateInfo sem_info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence_info = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    
    for (u32 i = 0; i < RAXTER_MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(ctx->device, &sem_info, NULL, &sync->image_available[i]) != VK_SUCCESS) return false;
        if (vkCreateSemaphore(ctx->device, &sem_info, NULL, &sync->render_finished[i]) != VK_SUCCESS) return false;
        if (vkCreateFence(ctx->device, &fence_info, NULL, &sync->in_flight[i]) != VK_SUCCESS) return false;
    }
    sync->current_frame = 0;
    return true;
}

void vk_sync_objects_destroy(vk_context* ctx, vk_sync_objects* sync) {
    for (u32 i = 0; i < RAXTER_MAX_FRAMES_IN_FLIGHT; i++) {
        if (sync->image_available[i]) vkDestroySemaphore(ctx->device, sync->image_available[i], NULL);
        if (sync->render_finished[i]) vkDestroySemaphore(ctx->device, sync->render_finished[i], NULL);
        if (sync->in_flight[i]) vkDestroyFence(ctx->device, sync->in_flight[i], NULL);
    }
    memset(sync, 0, sizeof(vk_sync_objects));
}

bool vk_debug_create(vk_context* ctx, vk_debug* debug) {
    debug->enabled = false;
    
    PFN_vkCreateDebugUtilsMessengerEXT create_func = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(ctx->instance, "vkCreateDebugUtilsMessengerEXT");
    if (!create_func) return false;
    
    VkDebugUtilsMessengerCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
    };
    
    VkResult result = create_func(ctx->instance, &create_info, NULL, &debug->messenger);
    if (result == VK_SUCCESS) debug->enabled = true;
    return debug->enabled;
}

void vk_debug_destroy(vk_context* ctx, vk_debug* debug) {
    if (debug->enabled) {
        PFN_vkDestroyDebugUtilsMessengerEXT destroy_func = 
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(ctx->instance, "vkDestroyDebugUtilsMessengerEXT");
        if (destroy_func) destroy_func(ctx->instance, debug->messenger, NULL);
        debug->enabled = false;
    }
}

VkCommandBuffer vk_begin_single_time_commands(vk_context* ctx, VkCommandPool pool) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(ctx->device, &alloc_info, &cmd);
    
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &begin_info);
    return cmd;
}

void vk_end_single_time_commands(vk_context* ctx, VkCommandPool pool, VkCommandBuffer cmd, VkQueue queue) {
    vkEndCommandBuffer(cmd);
    
    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };
    vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(ctx->device, pool, 1, &cmd);
}

u32 vk_find_memory_type(vk_context* ctx, u32 type_filter, VkMemoryPropertyFlags properties) {
    for (u32 i = 0; i < ctx->memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && 
            (ctx->memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return UINT32_MAX;
}