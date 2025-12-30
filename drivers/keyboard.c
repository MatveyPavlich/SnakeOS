/* Keyboard driver for PS/2 controller connected to i3259A (PIC) */

#include "kprint.h"
#include "interrupt.h"
#include "util.h"
#include "stdint.h"
#include "stddef.h"
#include "cdev.h"
#include <stdbool.h>

#define PS2_DATA        0x60
#define PIC1_CMD        0x20
#define PIC1_DATA       0x21
#define KEYBUFFER_SIZE  128
#define KEYBOARD_IRQ    1

/* PS/2 key codes */
#define LEFT_SHIFT      0x2A
#define RIGHT_SHIFT     0x36
#define CAPS_LOCK       0x3A


/* Forward declarations */
static void keybuf_put(char c);
static unsigned char translate_scancode(uint8_t sc);
static size_t keyboard_read(void *buf, size_t n);
static void keyboard_irq_handler(int vector, struct interrupt_frame* frame,
                                 void * dev);

/* Private keyboard state and buffer */
static bool is_shift_enabled = false;
static bool is_capslock_enabled = false;

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
        if (irq_request(KEYBOARD_IRQ, keyboard_irq_handler, NULL)) {
                kprintf("Error: irq allocation for the keyboard failed\n");
                return 1;
        }

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

static void keyboard_irq_handler(int vector, struct interrupt_frame *frame,
                void *dev)
{
        (void)vector; (void)frame, (void)dev;

        /* Check if a key was pressed or released by looking up bit 7 (0x80)
         * in the scancode */
        uint8_t ps2_scancode = inb(PS2_DATA);
        bool is_key_pressed = !(code & 0x80);
        uint8_t key_code = ps2_scancode & 0x7F;

        switch (key_code) {
                case (LEFT_SHIFT || RIGHT_SHIFT):
                        is_shift_enabled = is_key_pressed;
                        break;
                case CAPS_LOCK:
                        /* Toggle capslock on the press only */
                        if (is_key_pressed)
                                caps_lock = !is_capslock_enabled;
                        break;
                default:
                        /* Handle general key on press only */
                        if (is_key_pressed)
                                keyboard_handle_key(pressed_key);
                        break;
        }
}
char unsigned pressed_key = translate_scancode(key_code);
keybuf_put((char)pressed_key);
static void keybuf_put(char c)
{
        int next = (keybuf_head + 1) % KEYBUFFER_SIZE;
        // check buffer not full
        if (next != keybuf_tail) {
                keybuf[keybuf_head] = c;
                keybuf_head = next;
        }
        // else: buffer full => drop key
}

/* Translate PS2 scancodes into characters */
static unsigned char translate_scancode(uint8_t scancode)
{
        if (scancode < sizeof(base_map)) {
                bool is_shifted = (shift_l || shift_r);
                unsigned char c = shifted ? shift_map[sc] : base_map[sc];

                if (c >= 'a' && c <= 'z') {
                        if (caps_lock && !shifted) c = (unsigned char)(c - 'a' + 'A');
                }
                else if (c >= 'A' && c <= 'Z') {
                        if (caps_lock && !shifted) c = (unsigned char)(c - 'A' + 'a');
                }
                return c;
        }
        return 0;
}

static size_t keyboard_read(void *buf, size_t n)
{
        char *out = buf;
        size_t i = 0;

        while (i < n) {
                /* Busy-wait until a key is available */
                while (keybuf_head == keybuf_tail) {
                        __asm__("hlt");
                }

                out[i++] = keybuf[keybuf_tail];
                keybuf_tail = (keybuf_tail + 1) % KEYBUFFER_SIZE;
        }

        return (size_t)i;
}





