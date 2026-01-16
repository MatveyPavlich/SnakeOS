/* input.h */
#pragma once
#include <stdint.h>

enum keycode {
        KEY_NONE = 0,

        /* Printable (logical) keys */
        KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
        KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N,
        KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
        KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
        KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

        /* Control */
        KEY_ENTER,
        KEY_BACKSPACE,
        KEY_TAB,
        KEY_ESC,

        /* Navigation */
        KEY_LEFT,
        KEY_RIGHT,
        KEY_UP,
        KEY_DOWN,
        KEY_HOME,
        KEY_END,

        /* Modifiers (reported separately too) */
        KEY_SHIFT,
        KEY_CTRL,
        KEY_ALT,
};

enum key_action {
        KEY_PRESS,
        KEY_RELEASE,
};

#define MOD_SHIFT  (1 << 0)
#define MOD_CTRL   (1 << 1)
#define MOD_ALT    (1 << 2)

struct key_event {
        enum keycode     key;
        enum key_action  action;
        uint8_t          mods;
        char             ascii;
};

