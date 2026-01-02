/* Keyboard module to evaluate and store keys for consumption by tty */

#include "cdev.h"
#include "i8042.h"
#include "kprint.h"
#include <stdbool.h>
#include "stddef.h"
#include "stdint.h"
#include "util.h"

#define KEYBUFFER_SIZE  128
#define KEYBOARD_IRQ    1

/* PS/2 key codes */
#define LEFT_SHIFT      0x2A
#define RIGHT_SHIFT     0x36
#define CAPS_LOCK       0x3A

/* Forward declarations */
static void keybuf_put(char c);
static unsigned char translate_scancode(uint8_t sc);
size_t keyboard_read(void *buffer, size_t n);

struct keyboard_state {
        bool shift;
        bool caps;
};

/* Private keyboard state and buffer */
static struct keyboard_state kbd;
static char key_buffer[KEYBUFFER_SIZE]; /* ring buffer for charactes */
static int key_buffer_head = 0;
static int key_buffer_tail = 0;

static struct cdev keyboard;
static struct cdev_ops keyboard_ops = {
        .read = &keyboard_read,
        .write = NULL,
        .ioctl = NULL,
};

static const unsigned char base_map[0x60] = {
        /*00*/  0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
        /*0F*/ '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
        /*1E*/ 'a','s','d','f','g','h','j','k','l',';','\'','`',  0,'\\','z',
        /*2D*/ 'x','c','v','b','n','m',',','.','/',  0,  '*',  0,' ',
        /*39*/  ' ',
        /*3A..*/  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        /*44*/  0,   0,   0,   0,   0,   0
};

static const unsigned char shift_map[0x60] = {
        /*00*/  0,  27,'!','@','#','$','%','^','&','*','(',')','_','+', '\b',
        /*0F*/ '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
        /*1E*/ 'A','S','D','F','G','H','J','K','L',':','"','~',  0, '|','Z',
        /*2D*/ 'X','C','V','B','N','M','<','>','?',  0,  '*',  0,' ',
        /*39*/  ' ', 
        /*3A..*/  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        /*44*/  0,   0,   0,   0,   0,   0
};

int keyboard_init(void)
{
        i8042_init(); /* PS/2 controller for the keyboard */

        keyboard = (struct cdev) {
                .name = "keyboard",
                .ops  = &keyboard_ops,
                .priv = NULL,
        };

        if (cdev_register(&keyboard)) {
                kprintf("Error: cdev registration for keyboard failed\n");
                return 1;
        }

        return 0;
}

/* keyboard_read - caller method for reading keyboard buffer.
 * @buffer:        pointer to the buffer where to copy characters.
 * @n:             number of characters to copy.
 * Known bug: data race is possible if keyboard_read is executed while another
 * keyboard interrupt is fired and new charactes are appended to the buffer
 */
size_t keyboard_read(void *buffer, size_t n)
{
        char *out = buffer;
        size_t i = 0;

        while (i < n) {
                /* Busy-wait until a key is available */
                while (key_buffer_head == key_buffer_tail) {
                        __asm__ volatile ("hlt");
                }

                out[i++] = key_buffer[key_buffer_tail];
                key_buffer_tail = (key_buffer_tail + 1) % KEYBUFFER_SIZE;
        }

        return (size_t)i;
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
                        return;
                case CAPS_LOCK:
                        /* Toggle capslock on the press only */
                        if (is_key_pressed)
                                kbd.caps = !kbd.caps;
                        return;
                default:
                        /* Do not buffer released keys, just the pressed ones */
                        if (!is_key_pressed)
                                return;
                        break;
        }
        char unsigned pressed_key = translate_scancode(key_scancode);
        keybuf_put((char)pressed_key);
}

/* Translate PS2 scancodes into characters */
static unsigned char translate_scancode(uint8_t sc)
{
        if (sc < sizeof(base_map)) {
                unsigned char c = kbd.shift ? shift_map[sc] : base_map[sc];

                if (c >= 'a' && c <= 'z') {
                        if (kbd.caps && !kbd.shift)
                                c = (unsigned char)(c - 'a' + 'A');
                }
                else if (c >= 'A' && c <= 'Z') {
                        if (kbd.caps && !kbd.shift)
                                c = (unsigned char)(c - 'A' + 'a');
                }
                return c;
        }
        return 0;
}

static void keybuf_put(char c)
{
        int next = (key_buffer_head + 1) % KEYBUFFER_SIZE;
        /* check buffer not full */
        if (next != key_buffer_tail) {
                key_buffer[key_buffer_head] = c;
                key_buffer_head = next;
        }
        /* else: buffer full => drop key */
}
