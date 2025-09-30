#include "kprint.h"
#include "isr.h"
#include "util.h"

extern void keyboard_handler();
extern void print_clock(uint64_t* tick_pointer);

static isrptr_t interrupt_handlers[256];
uint64_t tick = 0;


static void timer_callback(int vector, struct interrupt_frame* frame) {
    (void)vector; (void)frame;         // unused
    tick++;
    if (tick % 100 == 0) print_clock(&tick); // ~1 second if PIT set to 100 Hz
    outb(0x20, 0x20);                  // Send EOI to PIC
}

void register_interrupt_handler(int vector, isrptr_t handler) {
    if (vector < 0 || vector >= 256) {
        // out of range — could panic or ignore
        return;
    }
    interrupt_handlers[vector] = handler;
}

void initTimer(uint32_t frequency) {
    register_interrupt_handler(32, timer_callback);
    uint32_t divisor = 1193182 / frequency;

    // Command byte: channel 0, lobyte/hibyte, mode 3 (square wave)
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);         // low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // high byte
}

void gp_fault_handler(int vector, struct interrupt_frame* frame) {
    kprintf("General Protection Fault: %d\n", vector);
    kprintf("RIP = %x\n",frame->rip);
    kprintf("System halted\n");

    while (1) __asm__("hlt");
}

/* =================== isrHandler =================== 
// Called by all ISR stubs (index pushed by asm stub)
// ================================================*/

void isrHandler(int index, struct interrupt_frame* frame) {

    // register_interrupt_handler(32, keyboard_handler); 
    
    // Call ISR if ptr not empty
    if (interrupt_handlers[index])
        interrupt_handlers[index](index, frame);
    
    // Else print a general message
    else {
        kprintf("Unhandled interrupt: %d\n", index);
        kprintf("System halted.\n");
        while (1) __asm__("hlt");
    }
}