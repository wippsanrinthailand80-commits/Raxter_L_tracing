#ifndef RAXTER_INPUT_H
#define RAXTER_INPUT_H

#include "../core/raxter_types.h"

typedef enum {
    RAXTER_KEY_UNKNOWN = 0,
    RAXTER_KEY_ESCAPE,
    RAXTER_KEY_W, RAXTER_KEY_A, RAXTER_KEY_S, RAXTER_KEY_D,
    RAXTER_KEY_Q, RAXTER_KEY_E,
    RAXTER_KEY_UP, RAXTER_KEY_DOWN, RAXTER_KEY_LEFT, RAXTER_KEY_RIGHT,
    RAXTER_KEY_SPACE,
    RAXTER_KEY_LEFT_SHIFT, RAXTER_KEY_LEFT_CTRL,
    RAXTER_KEY_1, RAXTER_KEY_2, RAXTER_KEY_3, RAXTER_KEY_4, RAXTER_KEY_5,
    RAXTER_KEY_6, RAXTER_KEY_7, RAXTER_KEY_8, RAXTER_KEY_9, RAXTER_KEY_0,
    RAXTER_KEY_F1, RAXTER_KEY_F2, RAXTER_KEY_F3, RAXTER_KEY_F4,
    RAXTER_KEY_F5, RAXTER_KEY_F6, RAXTER_KEY_F7, RAXTER_KEY_F8,
    RAXTER_KEY_F9, RAXTER_KEY_F10, RAXTER_KEY_F11, RAXTER_KEY_F12,
    RAXTER_KEY_COUNT
} raxter_key;

typedef struct {
    bool keys[RAXTER_KEY_COUNT];
    bool keys_prev[RAXTER_KEY_COUNT];
    f32 mouse_x, mouse_y;
    f32 mouse_dx, mouse_dy;
    bool mouse_buttons[3];
    bool mouse_buttons_prev[3];
    f32 scroll_x, scroll_y;
} raxter_input;

void input_update(raxter_input* input);
bool input_key_pressed(raxter_input* input, raxter_key key);
bool input_key_released(raxter_input* input, raxter_key key);
bool input_key_down(raxter_input* input, raxter_key key);
bool input_mouse_pressed(raxter_input* input, int button);
bool input_mouse_released(raxter_input* input, int button);
bool input_mouse_down(raxter_input* input, int button);
vec2 input_mouse_delta(raxter_input* input);
vec2 input_mouse_position(raxter_input* input);

#endif