/* Keyboard module to convert keyboard scancodes into key events and send those
 * into the tty layer.*/

#include "input.h"
#include "i8042.h"
#include "keyboard.h"
#include "kprint.h"
#include <stdbool.h>
#include "stddef.h"
#include "stdint.h"
#include "tty.h"
#include "util.h"

#define KEYBOARD_IRQ    1

/* PS/2 key codes */
#define LEFT_SHIFT      0x2A
#define RIGHT_SHIFT     0x36
#define CAPS_LOCK       0x3A

/* State of modifier keys */
struct keyboard_state {
        bool shift;
        bool caps;
};
static struct keyboard_state kbd;

// static const unsigned char base_map[0x60] = {
//         /*00*/  0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
//         /*0F*/ '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
//         /*1E*/ 'a','s','d','f','g','h','j','k','l',';','\'','`',  0,'\\','z',
//         /*2D*/ 'x','c','v','b','n','m',',','.','/',  0,  '*',  0,' ',
//         /*39*/  ' ',
//         /*3A..*/  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
//         /*44*/  0,   0,   0,   0,   0,   0
// };
//
// static const unsigned char shift_map[0x60] = {
//         /*00*/  0,  27,'!','@','#','$','%','^','&','*','(',')','_','+', '\b',
//         /*0F*/ '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
//         /*1E*/ 'A','S','D','F','G','H','J','K','L',':','"','~',  0, '|','Z',
//         /*2D*/ 'X','C','V','B','N','M','<','>','?',  0,  '*',  0,' ',
//         /*39*/  ' ', 
//         /*3A..*/  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
//         /*44*/  0,   0,   0,   0,   0,   0
// };

int keyboard_init(void)
{
        i8042_init(); /* PS/2 controller for the keyboard */

        return 0;
}

static enum keycode scancode_to_keycode(uint8_t scancode)
{
        switch (scancode) {
                /* control keys */
                case 0x1C: return KEY_ENTER;
                case 0x0E: return KEY_BACKSPACE;
                case 0x4B: return KEY_LEFT;
                case 0x4D: return KEY_RIGHT;
                case 0x48: return KEY_UP;
                case 0x50: return KEY_DOWN;

                /* letters */
                case 0x1E: return KEY_A;
                case 0x30: return KEY_B;
                case 0x2E: return KEY_C;
                case 0x20: return KEY_D;
                case 0x12: return KEY_E;
                case 0x21: return KEY_F;
                case 0x22: return KEY_G;
                case 0x23: return KEY_H;
                case 0x17: return KEY_I;
                case 0x24: return KEY_J;
                case 0x25: return KEY_K;
                case 0x26: return KEY_L;
                case 0x32: return KEY_M;
                case 0x31: return KEY_N;
                case 0x18: return KEY_O;
                case 0x19: return KEY_P;
                case 0x10: return KEY_Q;
                case 0x13: return KEY_R;
                case 0x1F: return KEY_S;
                case 0x14: return KEY_T;
                case 0x16: return KEY_U;
                case 0x2F: return KEY_V;
                case 0x11: return KEY_W;
                case 0x2D: return KEY_X;
                case 0x15: return KEY_Y;
                case 0x2C: return KEY_Z;

                /* digits (top row, not keypad) */
                case 0x02: return KEY_1;
                case 0x03: return KEY_2;
                case 0x04: return KEY_3;
                case 0x05: return KEY_4;
                case 0x06: return KEY_5;
                case 0x07: return KEY_6;
                case 0x08: return KEY_7;
                case 0x09: return KEY_8;
                case 0x0A: return KEY_9;
                case 0x0B: return KEY_0;

                /* punctuation */
                case 0x0C: return KEY_MINUS;       /* - _ */
                case 0x0D: return KEY_EQUAL;       /* = + */
                case 0x1A: return KEY_LBRACKET;    /* [ { */
                case 0x1B: return KEY_RBRACKET;    /* ] } */
                case 0x27: return KEY_SEMICOLON;   /* ; : */
                case 0x28: return KEY_APOSTROPHE;  /* ' " */
                case 0x29: return KEY_GRAVE;       /* ` ~ */
                case 0x2B: return KEY_BACKSLASH;   /* \ | */
                case 0x33: return KEY_COMMA;       /* , < */
                case 0x34: return KEY_DOT;         /* . > */
                case 0x35: return KEY_SLASH;       /* / ? */
                case 0x39: return KEY_SPACE;

                default:
                        return KEY_NONE;
        }
}

static char keycode_to_ascii(enum keycode key, uint8_t mods, bool caps)
{
        bool shift = mods & MOD_SHIFT;

        /* letters */
        if (key >= KEY_A && key <= KEY_Z) {
                bool upper = shift ^ caps;
                return upper ? ('A' + key - KEY_A)
                             : ('a' + key - KEY_A);
        }

        /* digits (UK layout) */
        if (key >= KEY_0 && key <= KEY_9) {
                static const char unshifted[] = "0123456789";
                static const char shifted[]   = ")!\"£$%^&*(";
                return shift ? shifted[key - KEY_0]
                             : unshifted[key - KEY_0];
        }

        /* punctuation */
        switch (key) {
                case KEY_MINUS:       return shift ? '_' : '-';
                case KEY_EQUAL:       return shift ? '+' : '=';
                case KEY_LBRACKET:    return shift ? '{' : '[';
                case KEY_RBRACKET:    return shift ? '}' : ']';
                case KEY_SEMICOLON:   return shift ? ':' : ';';
                case KEY_APOSTROPHE:  return shift ? '"' : '\'';
                case KEY_GRAVE:       return shift ? '~' : '`';
                case KEY_BACKSLASH:   return shift ? '|' : '\\';
                case KEY_COMMA:       return shift ? '<' : ',';
                case KEY_DOT:         return shift ? '>' : '.';
                case KEY_SLASH:       return shift ? '?' : '/';
                case KEY_SPACE:       return ' ';
                default:
                        return 0;
        }
}

/* keyboard_handle_scancode - ingestion API for low-level drivers (i8042, USB)
 *                            to send scancodes to the keyboard module.
 * @scancode:                 scancode to be send.
 * TODO: figure out how to pass to the function the chip identity (currently
 *       PS/2 logic is hardcoded
 */
void keyboard_handle_scancode(uint8_t scancode)
{
        /* Check if a key was pressed or released by looking up bit 7 (0x80)
         * in the scancode for PS/2 */
        bool is_key_pressed = !(scancode & 0x80);
        uint8_t key_scancode = scancode & 0x7F;

        switch (key_scancode) {
                case LEFT_SHIFT:
                case RIGHT_SHIFT:
                        kbd.shift = is_key_pressed;
                        break;
                case CAPS_LOCK:
                        /* Toggle capslock on the press only */
                        if (is_key_pressed)
                                kbd.caps = !kbd.caps;
                        break;
        }

        enum keycode key = scancode_to_keycode(key_scancode);
        if (key == KEY_NONE)
                return;

        struct key_event ev = {
                .key    = key,
                .action = is_key_pressed ? KEY_PRESS : KEY_RELEASE,
                .mods   = kbd.shift ? MOD_SHIFT : 0,
                .ascii  = 0,
        };

        if (is_key_pressed)
                ev.ascii = keycode_to_ascii(key, ev.mods, kbd.caps);

        tty_handle_key(&ev);
}
