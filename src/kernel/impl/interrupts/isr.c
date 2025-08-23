#include "kprint.h"
#include "isr.h"

static isr_t interrupt_handlers[256];

void register_interrupt_handler(int n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// Called by all ISR stubs (vector pushed by stub)
void isrHandler(int vector) {
    if (interrupt_handlers[vector]) {
        interrupt_handlers[vector](vector);
    } else {
        kprint("Unhandled interrupt: ");

        // Print simple number
        char buf[4];
        buf[0] = '0' + (vector / 10);
        buf[1] = '0' + (vector % 10);
        buf[2] = '\n';
        buf[3] = 0;
        kprint(buf);
    }

    // If this was an IRQ, remember: send EOI to PIC (outb(0x20, 0x20))
}
