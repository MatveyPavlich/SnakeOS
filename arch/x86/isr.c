#include "kprint.h"
#include "isr.h"
#include "util.h"

static isrptr_t interrupt_handlers[256];

// // TODO: Migrate into IRQ core
// void register_interrupt_handler(int vector, isrptr_t handler)
// {
//         /* out of range — could panic or ignore */
//         if (vector < 0 || vector >= 256) return;
//
//         interrupt_handlers[vector] = handler;
// }
//
// void gp_fault_handler(int vector, struct interrupt_frame* frame)
// {
//         kprintf("General Protection Fault: %d\n", vector);
//         kprintf("RIP = %x\n",frame->rip);
//         kprintf("System halted\n");
//
//         while (1) __asm__("hlt");
// }

/* Called by all ISR stubs (index pushed by asm stub) */
void isr_handler(int index, struct interrupt_frame* frame)
{
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
