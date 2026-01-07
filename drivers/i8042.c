#include "interrupt.h"
#include "kprint.h"
#include "util.h"
#include "keyboard.h"

#define I8042_DATA         0x60
#define I8042_KEYBOARD_IRQ 1

static void i8042_irq_handler(int irq, struct interrupt_frame *frame,
                              void *dev);

int i8042_init(void)
{
        if (irq_request(I8042_KEYBOARD_IRQ, i8042_irq_handler, NULL)) {
                kprint("Error: irq allocation for PS/2 controller\n");
                return 1;
        }

        return 0;
}

static void i8042_irq_handler(int irq, struct interrupt_frame *frame,
                              void *dev)
{
        (void)irq; (void)frame, (void)dev;
        uint8_t i8042_scancode = inb(I8042_DATA);
        keyboard_handle_scancode(i8042_scancode);
}
