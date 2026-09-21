#include "core/vk_core.h"
#include <stdio.h>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

bool test_vk_context_create() {
    vk_context ctx;
    bool result = vk_context_create(&ctx, "RAXTER Test", false);
    TEST_ASSERT(result, "vk_context_create");
    
    if (result) {
        TEST_ASSERT(ctx.instance != VK_NULL_HANDLE, "instance created");
        TEST_ASSERT(ctx.physical_device != VK_NULL_HANDLE, "physical device selected");
        TEST_ASSERT(ctx.device != VK_NULL_HANDLE, "logical device created");
        TEST_ASSERT(ctx.graphics_queue != VK_NULL_HANDLE, "graphics queue");
        TEST_ASSERT(ctx.compute_queue != VK_NULL_HANDLE, "compute queue");
        
        printf("Device: %s\n", ctx.properties.deviceName);
        printf("API Version: %d.%d.%d\n", 
               VK_VERSION_MAJOR(ctx.properties.apiVersion),
               VK_VERSION_MINOR(ctx.properties.apiVersion),
               VK_VERSION_PATCH(ctx.properties.apiVersion));
        
        vk_context_destroy(&ctx);
    }
    
    printf("PASS: test_vk_context_create\n");
    return true;
}

bool test_vk_debug() {
    vk_context ctx;
    if (!vk_context_create(&ctx, "RAXTER Debug Test", true)) return false;
    
    vk_debug debug;
    bool result = vk_debug_create(&ctx, &debug);
    if (result) {
        printf("Debug messenger created\n");
        TEST_ASSERT(debug.enabled, "debug enabled");
        vk_debug_destroy(&ctx, &debug);
    } else {
        printf("Debug messenger not available (expected on some systems)\n");
    }
    
    vk_context_destroy(&ctx);
    printf("PASS: test_vk_debug\n");
    return true;
}

bool test_vk_memory() {
    vk_context ctx;
    if (!vk_context_create(&ctx, "RAXTER Memory Test", false)) return false;
    
    vk_buffer buffer;
    VkDeviceSize size = 1024 * 1024;
    bool result = vk_buffer_create(&ctx, &buffer, size, 
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    TEST_ASSERT(result, "vk_buffer_create");
    TEST_ASSERT(buffer.buffer != VK_NULL_HANDLE, "buffer handle");
    TEST_ASSERT(buffer.memory != VK_NULL_HANDLE, "memory handle");
    TEST_ASSERT(buffer.size == size, "buffer size");
    
    vk_buffer_destroy(&ctx, &buffer);
    vk_context_destroy(&ctx);
    
    printf("PASS: test_vk_memory\n");
    return true;
}

bool test_vk_sync() {
    vk_context ctx;
    if (!vk_context_create(&ctx, "RAXTER Sync Test", false)) return false;
    
    vk_sync_objects sync;
    bool result = vk_sync_objects_create(&ctx, &sync);
    TEST_ASSERT(result, "vk_sync_objects_create");
    
    for (int i = 0; i < RAXTER_MAX_FRAMES_IN_FLIGHT; i++) {
        TEST_ASSERT(sync.image_available[i] != VK_NULL_HANDLE, "image_available semaphore");
        TEST_ASSERT(sync.render_finished[i] != VK_NULL_HANDLE, "render_finished semaphore");
        TEST_ASSERT(sync.in_flight[i] != VK_NULL_HANDLE, "in_flight fence");
    }
    
    vk_sync_objects_destroy(&ctx, &sync);
    vk_context_destroy(&ctx);
    
    printf("PASS: test_vk_sync\n");
    return true;
}

int main() {
    int passed = 0;
    int total = 0;
    
    total++; if (test_vk_context_create()) passed++;
    total++; if (test_vk_debug()) passed++;
    total++; if (test_vk_memory()) passed++;
    total++; if (test_vk_sync()) passed++;
    
    printf("\n=== VK Core Tests: %d/%d passed ===\n", passed, total);
    return passed == total ? 0 : 1;
}