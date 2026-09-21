#include "input.h"
#include "window.h"
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <string.h>

static raxter_key x11_key_to_raxter(KeySym keysym) {
    switch (keysym) {
        case XK_Escape: return RAXTER_KEY_ESCAPE;
        case XK_w: case XK_W: return RAXTER_KEY_W;
        case XK_a: case XK_A: return RAXTER_KEY_A;
        case XK_s: case XK_S: return RAXTER_KEY_S;
        case XK_d: case XK_D: return RAXTER_KEY_D;
        case XK_q: case XK_Q: return RAXTER_KEY_Q;
        case XK_e: case XK_E: return RAXTER_KEY_E;
        case XK_Up: return RAXTER_KEY_UP;
        case XK_Down: return RAXTER_KEY_DOWN;
        case XK_Left: return RAXTER_KEY_LEFT;
        case XK_Right: return RAXTER_KEY_RIGHT;
        case XK_space: return RAXTER_KEY_SPACE;
        case XK_Shift_L: return RAXTER_KEY_LEFT_SHIFT;
        case XK_Control_L: return RAXTER_KEY_LEFT_CTRL;
        case XK_1: return RAXTER_KEY_1;
        case XK_2: return RAXTER_KEY_2;
        case XK_3: return RAXTER_KEY_3;
        case XK_4: return RAXTER_KEY_4;
        case XK_5: return RAXTER_KEY_5;
        case XK_6: return RAXTER_KEY_6;
        case XK_7: return RAXTER_KEY_7;
        case XK_8: return RAXTER_KEY_8;
        case XK_9: return RAXTER_KEY_9;
        case XK_0: return RAXTER_KEY_0;
        case XK_F1: return RAXTER_KEY_F1;
        case XK_F2: return RAXTER_KEY_F2;
        case XK_F3: return RAXTER_KEY_F3;
        case XK_F4: return RAXTER_KEY_F4;
        case XK_F5: return RAXTER_KEY_F5;
        case XK_F6: return RAXTER_KEY_F6;
        case XK_F7: return RAXTER_KEY_F7;
        case XK_F8: return RAXTER_KEY_F8;
        case XK_F9: return RAXTER_KEY_F9;
        case XK_F10: return RAXTER_KEY_F10;
        case XK_F11: return RAXTER_KEY_F11;
        case XK_F12: return RAXTER_KEY_F12;
        default: return RAXTER_KEY_UNKNOWN;
    }
}

static raxter_input g_input = {0};

void input_update(raxter_input* input) {
    if (!input) input = &g_input;
    
    memcpy(input->keys_prev, input->keys, sizeof(input->keys));
    memcpy(input->mouse_buttons_prev, input->mouse_buttons, sizeof(input->mouse_buttons));
    input->mouse_dx = 0.0f;
    input->mouse_dy = 0.0f;
    input->scroll_x = 0.0f;
    input->scroll_y = 0.0f;
    
    XEvent event;
    while (XPending(x11_display)) {
        XNextEvent(x11_display, &event);
        switch (event.type) {
            case KeyPress: {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                raxter_key k = x11_key_to_raxter(key);
                if (k != RAXTER_KEY_UNKNOWN) input->keys[k] = true;
                break;
            }
            case KeyRelease: {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                raxter_key k = x11_key_to_raxter(key);
                if (k != RAXTER_KEY_UNKNOWN) input->keys[k] = false;
                break;
            }
            case ButtonPress: {
                if (event.xbutton.button >= 1 && event.xbutton.button <= 3) {
                    input->mouse_buttons[event.xbutton.button - 1] = true;
                }
                if (event.xbutton.button == 4) input->scroll_y += 1.0f;
                if (event.xbutton.button == 5) input->scroll_y -= 1.0f;
                break;
            }
            case ButtonRelease: {
                if (event.xbutton.button >= 1 && event.xbutton.button <= 3) {
                    input->mouse_buttons[event.xbutton.button - 1] = false;
                }
                break;
            }
            case MotionNotify: {
                f32 new_x = (f32)event.xmotion.x;
                f32 new_y = (f32)event.xmotion.y;
                input->mouse_dx = new_x - input->mouse_x;
                input->mouse_dy = new_y - input->mouse_y;
                input->mouse_x = new_x;
                input->mouse_y = new_y;
                break;
            }
        }
    }
}

bool input_key_pressed(raxter_input* input, raxter_key key) {
    if (!input) input = &g_input;
    return input->keys[key] && !input->keys_prev[key];
}

bool input_key_released(raxter_input* input, raxter_key key) {
    if (!input) input = &g_input;
    return !input->keys[key] && input->keys_prev[key];
}

bool input_key_down(raxter_input* input, raxter_key key) {
    if (!input) input = &g_input;
    return input->keys[key];
}

bool input_mouse_pressed(raxter_input* input, int button) {
    if (!input) input = &g_input;
    if (button < 0 || button >= 3) return false;
    return input->mouse_buttons[button] && !input->mouse_buttons_prev[button];
}

bool input_mouse_released(raxter_input* input, int button) {
    if (!input) input = &g_input;
    if (button < 0 || button >= 3) return false;
    return !input->mouse_buttons[button] && input->mouse_buttons_prev[button];
}

bool input_mouse_down(raxter_input* input, int button) {
    if (!input) input = &g_input;
    if (button < 0 || button >= 3) return false;
    return input->mouse_buttons[button];
}

vec2 input_mouse_delta(raxter_input* input) {
    if (!input) input = &g_input;
    return (vec2){input->mouse_dx, input->mouse_dy};
}

vec2 input_mouse_position(raxter_input* input) {
    if (!input) input = &g_input;
    return (vec2){input->mouse_x, input->mouse_y};
}