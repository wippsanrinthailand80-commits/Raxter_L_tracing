#include "renderer/renderer.h"
#include "platform/window.h"
#include "platform/input.h"
#include "platform/platform.h"
#include "math/vec.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    
    const u32 WIDTH = 1280;
    const u32 HEIGHT = 720;
    const u32 MAX_BOUNCES = 8;
    const bool VALIDATION = true;
    
    raxter_window window;
    if (!platform_window_create(&window, "RAXTER Wave Optics Engine", WIDTH, HEIGHT)) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }
    
    raxter_renderer renderer;
    if (!raxter_renderer_create(&renderer, "Raxter L Tracing", WIDTH, HEIGHT, MAX_BOUNCES, VALIDATION)) {
        fprintf(stderr, "Failed to create renderer\n");
        platform_window_destroy(&window);
        return 1;
    }
    
    raxter_input input = {0};
    f64 last_time = platform_get_time();
    
    printf("RAXTER Engine Started\n");
    printf("Render modes:\n");
    printf("  1 - Wave Optics\n");
    printf("  2 - Interference\n");
    printf("  3 - Diffraction\n");
    printf("  4 - Polarization\n");
    printf("  5 - Light Propagation\n");
    printf("ESC to quit\n");
    
    while (!window.should_close) {
        f64 current_time = platform_get_time();
        f32 delta_time = (f32)(current_time - last_time);
        last_time = current_time;
        
        platform_window_poll_events(&window);
        input_update(&input);
        
        if (input_key_pressed(&input, RAXTER_KEY_ESCAPE)) {
            window.should_close = true;
        }
        if (input_key_pressed(&input, RAXTER_KEY_1)) raxter_renderer_set_mode(&renderer, RAXTER_RENDER_MODE_WAVE_OPTICS);
        if (input_key_pressed(&input, RAXTER_KEY_2)) raxter_renderer_set_mode(&renderer, RAXTER_RENDER_MODE_INTERFERENCE);
        if (input_key_pressed(&input, RAXTER_KEY_3)) raxter_renderer_set_mode(&renderer, RAXTER_RENDER_MODE_DIFFRACTION);
        if (input_key_pressed(&input, RAXTER_KEY_4)) raxter_renderer_set_mode(&renderer, RAXTER_RENDER_MODE_POLARIZATION);
        if (input_key_pressed(&input, RAXTER_KEY_5)) raxter_renderer_set_mode(&renderer, RAXTER_RENDER_MODE_LIGHT_PROPAGATION);
        if (input_key_pressed(&input, RAXTER_KEY_UP)) {
            renderer.max_bounces = RAXTER_MIN(renderer.max_bounces + 1, 8);
            printf("Max bounces: %u\n", renderer.max_bounces);
        }
        if (input_key_pressed(&input, RAXTER_KEY_DOWN)) {
            renderer.max_bounces = RAXTER_MAX(renderer.max_bounces - 1, 1);
            printf("Max bounces: %u\n", renderer.max_bounces);
        }
        
        raxter_renderer_update(&renderer, delta_time);
        
        if (raxter_renderer_begin_frame(&renderer)) {
            raxter_renderer_end_frame(&renderer);
        }
    }
    
    vkDeviceWaitIdle(renderer.vk.device);
    raxter_renderer_destroy(&renderer);
    platform_window_destroy(&window);
    
    printf("RAXTER Engine Shutdown\n");
    return 0;
}