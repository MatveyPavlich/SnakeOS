#include "kprint.h"
#include "isr.h"
#include "util.h"

static isrptr_t interrupt_handlers[256];
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

void register_interrupt_handler(int vector, isrptr_t handler) {
    if (vector < 0 || vector >= 256) {
        // out of range — could panic or ignore
        return;
    }
    interrupt_handlers[vector] = handler;
}

void initTimer(uint32_t frequency) {

    // PIT runs at 1193182 Hz
    uint32_t divisor = 1193182 / frequency;

    // Command byte: channel 0, lobyte/hibyte, mode 3 (square wave)
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);         // low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // high byte
}

void gp_fault_handler(int vector) {
    kprint("General Protection Fault!\n");
    while (1) __asm__("hlt");
}

/* =================== isrHandler =================== 
// Called by all ISR stubs (index pushed by asm stub)
// ================================================*/

void isrHandler(int index) {

    register_interrupt_handler(13, gp_fault_handler);   // GP fault
    register_interrupt_handler(32, timer_callback); 
    
    // Call ISR if ptr not empty
    if (interrupt_handlers[index]) {
        interrupt_handlers[index](index);
    } 
    
    // Else print a general message
    else {
        kprint("Unhandled interrupt: ");
        char interrupt_number[4];
        interrupt_number[0] = '0' + (index / 10);
        interrupt_number[1] = '0' + (index % 10);
        interrupt_number[2] = '\n';
        interrupt_number[3] = 0;
        kprint(interrupt_number);
        kprint("System halted.\n");
        while (1) __asm__("hlt");
    }

    // If this was an IRQ, remember: send EOI to PIC (outb(0x20, 0x20))
}