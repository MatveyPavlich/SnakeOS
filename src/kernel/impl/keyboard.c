#include "kprint.h"
#include "isr.h"
#include "util.h"
#include "stdint.h"
#include <stdbool.h>

/* ===== PS/2 + PIC ===== */
#define PS2_DATA        0x60
#define PIC1_CMD        0x20
#define PIC1_DATA       0x21
#define PIC_EOI         0x20

/* IRQ1 (keyboard) => vector 32 + 1 = 33 (with PIC remapped to 0x20) */
#define IRQ_KEYBOARD_VECTOR  (32 + 1)

/* ===== State ===== */
static bool shift_l = false;
static bool shift_r = false;
static bool caps_lock = false;

/* ===== Scancode set 1 (make codes), minimal tables ===== */
/* Index by scancode (without 0x80 break bit). You can extend as needed. */
static const unsigned char base_map[0x60] = {
/*00*/  0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
/*0F*/ '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
/*1E*/ 'a','s','d','f','g','h','j','k','l',';','\'','`',  0,'\\','z',
/*2D*/ 'x','c','v','b','n','m',',','.','/',  0,  '*',  0,' ',
/*39*/  ' ', /* 0x39 Space (dup so we’re safe) */
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

/* ===== Translate scancode -> ASCII, with Shift + CapsLock for letters ===== */
static unsigned char translate_scancode(uint8_t sc) {
    if (sc < sizeof(base_map)) {
        bool shifted = (shift_l || shift_r);
        unsigned char c = shifted ? shift_map[sc] : base_map[sc];

        /* CapsLock only flips case for letters */
        if (c >= 'a' && c <= 'z') {
            if (caps_lock && !shifted) c = (unsigned char)(c - 'a' + 'A');
        } else if (c >= 'A' && c <= 'Z') {
            if (caps_lock && !shifted) c = (unsigned char)(c - 'A' + 'a');
        }
        return c;
    }
    return 0;
}

/* ===== Keyboard IRQ handler ===== */
static void keyboard_handler(int vector, struct interrupt_frame* frame) {
    (void)vector; (void)frame;

    /* Read scancode once */
    uint8_t code = inb(PS2_DATA);
    bool is_break = (code & 0x80) != 0;
    uint8_t make = code & 0x7F;

    switch (make) {
        case 0x2A: /* Left Shift */
            shift_l = !is_break;
            break;
        case 0x36: /* Right Shift */
            shift_r = !is_break;
            break;
        case 0x3A: /* CapsLock toggles on make only */
            if (!is_break) caps_lock = !caps_lock;
            break;
        default:
            if (!is_break) {
                unsigned char ch = translate_scancode(make);
                if (ch) {
                    char s[2] = { (char)ch, 0 };
                    kprintf("%s", s);
                }
            }
            break;
    }

    /* Send EOI to PIC (master) */
    outb(PIC1_CMD, PIC_EOI);
}

/* ===== Init: register ISR and unmask IRQ1 on PIC ===== */
void init_keyboard(void) {
    register_interrupt_handler(IRQ_KEYBOARD_VECTOR, keyboard_handler);

    /* Unmask IRQ1 (bit1) on PIC1 */
    uint8_t mask = inb(PIC1_DATA);
    mask &= ~(1 << 1);
    outb(PIC1_DATA, mask);

    /* Optional: drain any pending byte from the controller */
    /* (Not strictly required in QEMU/Bochs) */
    /* while (inb(0x64) & 0x01) (void)inb(0x60); */
}
