// Exceptions subsystem

#include "exception.h"
#include "kprint.h"

/* Forward declarations */
static void gp_fault_handler(int vector, struct interrupt_frame* frame);
static void div_by_zero_handler(int vector, struct interrupt_frame* frame);


void exception_handle(int vector, struct interrupt_frame *frame)
{
        switch (vector) {
        case 0:
                div_by_zero_handler(vector, frame);
                break;
        case 13:
                gp_fault_handler(vector, frame);
                break;
        default:
                kprint("Exception %d unhandled. System halted\n", vector);
                while (1) __asm__("hlt");
        }
}

/* Handler for the General Protection (GP) fault */
static void gp_fault_handler(int vector, struct interrupt_frame* frame)
{
        kprint("General Protection Fault: %d\n", vector);
        kprint("RIP = %x\n",frame->rip);
        kprint("System halted\n");
        while (1) __asm__("hlt");
}

/* Division by zero */ 
static void div_by_zero_handler(int vector, struct interrupt_frame* frame)
{
        kprint("Division by zero: (exception %d)\n", vector);
        kprint("RIP = %x\n",frame->rip);
        kprint("System halted\n");
        while (1) __asm__("hlt");
}
