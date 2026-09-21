#ifndef RAXTER_WINDOW_H
#define RAXTER_WINDOW_H

#include "../core/raxter_types.h"
#include <vulkan/vulkan.h>

typedef struct {
    void* native_window;
    void* native_display;
    u32 width;
    u32 height;
    const char* title;
    bool should_close;
} raxter_window;

extern Display* x11_display;
extern Window x11_window;
extern xcb_connection_t* xcb_connection;
extern xcb_window_t xcb_window;

bool platform_window_create(raxter_window* window, const char* title, u32 width, u32 height);
void platform_window_destroy(raxter_window* window);
void platform_window_poll_events(raxter_window* window);
VkSurfaceKHR platform_create_surface(VkInstance instance, u32 width, u32 height);

#endif