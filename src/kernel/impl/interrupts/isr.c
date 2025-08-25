#include "kprint.h"
#include "isr.h"

static isrptr_t interrupt_handlers[256];

/* =================== isrHandler =================== 
// Called by all ISR stubs (index pushed by asm stub)
// ================================================*/

void isrHandler(int index) {
    
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
    }

    // If this was an IRQ, remember: send EOI to PIC (outb(0x20, 0x20))
}
