#include "compute/wave_optics.h"
#include "core/vk_core.h"
#include <stdio.h>
#include <time.h>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

bool bench_wave_optics() {
    vk_context ctx;
    if (!vk_context_create(&ctx, "RAXTER Bench", false)) return false;
    
    wave_optics_renderer renderer;
    if (!wave_optics_create(&ctx, &renderer, 1920, 1080, 8)) {
        vk_context_destroy(&ctx);
        return false;
    }
    
    vk_command_pool pool;
    vk_command_pool_create(&ctx, &pool, ctx.compute_queue_family, 0);
    
    VkCommandBuffer cmd = vk_begin_single_time_commands(&ctx, pool.pool);
    
    const int WARMUP = 10;
    const int ITERATIONS = 100;
    
    for (int i = 0; i < WARMUP; i++) {
        wave_optics_render(&ctx, &renderer, cmd, 0);
    }
    vkEndCommandBuffer(cmd);
    vkQueueSubmit(ctx.compute_queue, 1, &(VkSubmitInfo){.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.commandBufferCount=1,.pCommandBuffers=&cmd}, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx.compute_queue);
    
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        cmd = vk_begin_single_time_commands(&ctx, pool.pool);
        wave_optics_render(&ctx, &renderer, cmd, 0);
        vkEndCommandBuffer(cmd);
        vkQueueSubmit(ctx.compute_queue, 1, &(VkSubmitInfo){.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.commandBufferCount=1,.pCommandBuffers=&cmd}, VK_NULL_HANDLE);
        vkQueueWaitIdle(ctx.compute_queue);
    }
    double end = get_time_ms();
    
    double total_ms = end - start;
    double avg_ms = total_ms / ITERATIONS;
    double fps = 1000.0 / avg_ms;
    
    printf("Wave Optics Benchmark (1920x1080, 8 bounces):\n");
    printf("  Iterations: %d\n", ITERATIONS);
    printf("  Total time: %.2f ms\n", total_ms);
    printf("  Average: %.2f ms/frame\n", avg_ms);
    printf("  FPS: %.1f\n", fps);
    printf("  MPixels/s: %.1f\n", (1920.0 * 1080.0 * ITERATIONS) / (total_ms * 1000.0));
    
    vk_command_pool_destroy(&ctx, &pool);
    wave_optics_destroy(&ctx, &renderer);
    vk_context_destroy(&ctx);
    
    return true;
}

bool bench_interference() {
    vk_context ctx;
    if (!vk_context_create(&ctx, "RAXTER Bench Interference", false)) return false;
    
    interference_renderer renderer;
    if (!interference_create(&ctx, &renderer, 1920, 1080)) {
        vk_context_destroy(&ctx);
        return false;
    }
    
    vk_command_pool pool;
    vk_command_pool_create(&ctx, &pool, ctx.compute_queue_family, 0);
    
    VkCommandBuffer cmd = vk_begin_single_time_commands(&ctx, pool.pool);
    
    const int ITERATIONS = 1000;
    
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        cmd = vk_begin_single_time_commands(&ctx, pool.pool);
        interference_render(&ctx, &renderer, cmd, 0);
        vkEndCommandBuffer(cmd);
        vkQueueSubmit(ctx.compute_queue, 1, &(VkSubmitInfo){.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.commandBufferCount=1,.pCommandBuffers=&cmd}, VK_NULL_HANDLE);
        vkQueueWaitIdle(ctx.compute_queue);
    }
    double end = get_time_ms();
    
    double total_ms = end - start;
    double avg_ms = total_ms / ITERATIONS;
    double fps = 1000.0 / avg_ms;
    
    printf("Interference Benchmark (1920x1080):\n");
    printf("  Iterations: %d\n", ITERATIONS);
    printf("  Total time: %.2f ms\n", total_ms);
    printf("  Average: %.2f ms/frame\n", avg_ms);
    printf("  FPS: %.1f\n", fps);
    
    vk_command_pool_destroy(&ctx, &pool);
    interference_destroy(&ctx, &renderer);
    vk_context_destroy(&ctx);
    
    return true;
}

bool bench_diffraction() {
    vk_context ctx;
    if (!vk_context_create(&ctx, "RAXTER Bench Diffraction", false)) return false;
    
    diffraction_renderer renderer;
    if (!diffraction_create(&ctx, &renderer, 1920, 1080)) {
        vk_context_destroy(&ctx);
        return false;
    }
    
    vk_command_pool pool;
    vk_command_pool_create(&ctx, &pool, ctx.compute_queue_family, 0);
    
    VkCommandBuffer cmd = vk_begin_single_time_commands(&ctx, pool.pool);
    
    const int ITERATIONS = 1000;
    
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        cmd = vk_begin_single_time_commands(&ctx, pool.pool);
        diffraction_render(&ctx, &renderer, cmd, 0);
        vkEndCommandBuffer(cmd);
        vkQueueSubmit(ctx.compute_queue, 1, &(VkSubmitInfo){.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.commandBufferCount=1,.pCommandBuffers=&cmd}, VK_NULL_HANDLE);
        vkQueueWaitIdle(ctx.compute_queue);
    }
    double end = get_time_ms();
    
    double total_ms = end - start;
    double avg_ms = total_ms / ITERATIONS;
    double fps = 1000.0 / avg_ms;
    
    printf("Diffraction Benchmark (1920x1080):\n");
    printf("  Iterations: %d\n", ITERATIONS);
    printf("  Total time: %.2f ms\n", total_ms);
    printf("  Average: %.2f ms/frame\n", avg_ms);
    printf("  FPS: %.1f\n", fps);
    
    vk_command_pool_destroy(&ctx, &pool);
    diffraction_destroy(&ctx, &renderer);
    vk_context_destroy(&ctx);
    
    return true;
}

int main() {
    printf("=== RAXTER Wave Optics Benchmarks ===\n\n");
    
    bench_wave_optics();
    printf("\n");
    bench_interference();
    printf("\n");
    bench_diffraction();
    printf("\n");
    
    printf("=== Benchmarks Complete ===\n");
    return 0;
}