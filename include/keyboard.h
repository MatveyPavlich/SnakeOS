#include "stdint.h"

/* keyboard_init - initialise the keyboard module to allocate an IRQ line for
 *                 it
 */
int keyboard_init(void);

/* keyboard_handle_scancode - ingestion API for low-level drivers (i8042, USB)
 *                            to send scancodes to the keyboard module.
 * @scancode:                 scancode to be send.
 */
void keyboard_handle_scancode(uint8_t scancode);

/* TODO: later you can migrate keycode enum and struct into input.h since many
 *       things will be using those. E.g., USB HID keyboard, serial input,
 *       network consoles, etc.
 */
enum keycode {
        KEY_NONE = 0,
        KEY_CHAR,
        KEY_ENTER,
        KEY_BACKSPACE,
        KEY_LEFT,
        KEY_RIGHT,
        KEY_UP,
        KEY_DOWN,
};

struct key_event {
        enum keycode code;
        char ch;
};
