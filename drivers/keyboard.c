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

// State
static bool shift_l   = false;
static bool shift_r   = false;
static bool caps_lock = false;

// Key's in the buffer
static char keybuf[KEYBUFFER_SIZE];
static int keybuf_head = 0;
static int keybuf_tail = 0;

/* Forward declarations */
static size_t keyboard_read(void *buf, size_t n);
static void keyboard_irq_handler(int vector, struct interrupt_frame* frame,
                                 void * dev);

static struct cdev keyboard;
static struct cdev_ops keyboard_ops = {
        .read = &keyboard_read,
        .write = NULL,
        .ioctl = NULL,
};

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

static void keybuf_put(char c)
{
        int next = (keybuf_head + 1) % KEYBUFFER_SIZE;
        if (next != keybuf_tail) {   // check buffer not full
                keybuf[keybuf_head] = c;
                keybuf_head = next;
        }
        // else: buffer full => drop key
}

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

static unsigned char translate_scancode(uint8_t sc)
{
        if (sc < sizeof(base_map)) {
                bool shifted = (shift_l || shift_r);
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


// Keyboard IRQ handler
static void keyboard_irq_handler(int vector, struct interrupt_frame* frame,
                              void * dev)
{
        (void)vector; (void)frame;

        // Read scancode once
        uint8_t code = inb(PS2_DATA);
        bool is_break = (code & 0x80) != 0;
        uint8_t make = code & 0x7F;

        switch (make) {
                case 0x2A: // Left Shift
                        shift_l = !is_break;
                        break;
                case 0x36: // Right Shift
                        shift_r = !is_break;
                        break;
                case 0x3A: // CapsLock toggles on make only
                        if (!is_break)
                                caps_lock = !caps_lock;
                        break;
                default:
                        if (!is_break) {
                                unsigned char ch = translate_scancode(make);
                                if (ch)
                                        keybuf_put((char)ch);
                        }
                        break;
        }
}
