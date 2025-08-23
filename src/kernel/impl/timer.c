#include "stdint.h"
#include "kprint.h"
#include "ports.h"    // inb/outb helpers
#include "isr.h"      // ISR registration

static uint64_t tick = 0;

static void timer_callback(int vector) {
    (void)vector;  // unused
    tick++;

    if (tick % 100 == 0) {   // ~1 second if PIT set to 100 Hz
        kprint("x");
    }

    // Send EOI to PIC
    outb(0x20, 0x20);
}

void init_timer(uint32_t frequency) {
    // Register handler (IRQ0 = vector 32)
    register_interrupt_handler(32, timer_callback);

    // PIT runs at 1193182 Hz
    uint32_t divisor = 1193182 / frequency;

    // Command byte: channel 0, lobyte/hibyte, mode 3 (square wave)
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);         // low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // high byte
}
