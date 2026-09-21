#include "window.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_xlib.h>
#include <vulkan/vulkan_xcb.h>
#include <vulkan/vulkan_wayland.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>
#include <wayland-client.h>

Display* x11_display = NULL;
Window x11_window = 0;
xcb_connection_t* xcb_connection = NULL;
xcb_window_t xcb_window = 0;
struct wl_display* wl_display = NULL;
struct wl_surface* wl_surface = NULL;

bool platform_window_create(raxter_window* window, const char* title, u32 width, u32 height) {
    memset(window, 0, sizeof(raxter_window));
    window->width = width;
    window->height = height;
    window->title = title;
    window->should_close = false;
    
    x11_display = XOpenDisplay(NULL);
    if (!x11_display) {
        fprintf(stderr, "Failed to open X11 display\n");
        return false;
    }
    
    int screen = DefaultScreen(x11_display);
    Window root = RootWindow(x11_display, screen);
    
    XSetWindowAttributes attrs = {0};
    attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;
    
    x11_window = XCreateWindow(x11_display, root, 0, 0, width, height, 0,
                               CopyFromParent, InputOutput, CopyFromParent,
                               CWEventMask, &attrs);
    
    XStoreName(x11_display, x11_window, title);
    XMapWindow(x11_display, x11_window);
    
    xcb_connection = XGetXCBConnection(x11_display);
    if (xcb_connection) {
        xcb_window = x11_window;
    }
    
    window->native_display = x11_display;
    window->native_window = (void*)(uintptr_t)x11_window;
    
    return true;
}

void platform_window_destroy(raxter_window* window) {
    if (x11_display && x11_window) {
        XDestroyWindow(x11_display, x11_window);
        XCloseDisplay(x11_display);
    }
    x11_display = NULL;
    x11_window = 0;
    xcb_connection = NULL;
    xcb_window = 0;
    wl_display = NULL;
    wl_surface = NULL;
    memset(window, 0, sizeof(raxter_window));
}

void platform_window_poll_events(raxter_window* window) {
    XEvent event;
    while (XPending(x11_display)) {
        XNextEvent(x11_display, &event);
        switch (event.type) {
            case KeyPress: {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_Escape) window->should_close = true;
                break;
            }
            case ConfigureNotify:
                window->width = event.xconfigure.width;
                window->height = event.xconfigure.height;
                break;
            case ClientMessage:
                window->should_close = true;
                break;
        }
    }
}

VkSurfaceKHR platform_create_surface(VkInstance instance, u32 width, u32 height) {
    (void)width; (void)height;
    if (x11_display && x11_window) {
        VkXlibSurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .dpy = x11_display,
            .window = x11_window,
        };
        VkSurfaceKHR surface;
        if (vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &surface) == VK_SUCCESS) {
            return surface;
        }
    }
    
    if (xcb_connection && xcb_window) {
        VkXcbSurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
            .connection = xcb_connection,
            .window = xcb_window,
        };
        VkSurfaceKHR surface;
        if (vkCreateXcbSurfaceKHR(instance, &create_info, NULL, &surface) == VK_SUCCESS) {
            return surface;
        }
    }
    
    if (wl_display && wl_surface) {
        VkWaylandSurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .display = wl_display,
            .surface = wl_surface,
        };
        VkSurfaceKHR surface;
        if (vkCreateWaylandSurfaceKHR(instance, &create_info, NULL, &surface) == VK_SUCCESS) {
            return surface;
        }
    }
    
    return VK_NULL_HANDLE;
}