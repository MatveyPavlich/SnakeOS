#pragma once
#include "stdint.h"

// API interface for irq module

struct interrupt_frame {
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t error_code;
};

/* irq_chip - structure to represent all properties of an IRQ chip
 * @name:            name of the chip
 * @irq_init:        int the chip
 * @irq_disable_all: disabling all interrupts to prevent them going into
 *                   the buffer
 * @irq_unmask:      unmask a specific irq
 * @irq_eoi:         send an End Of Interrupt (EOI) signal to irq_chip
 */
struct irq_chip {
        const char *name;
        void (*irq_init)(void);
        void (*irq_disable_all)(void);
        void (*irq_unmask)(int irq);
        void (*irq_eoi)(int irq);
};

typedef void (*irq_handler_t)(struct interrupt_frame *);
int register_irq(int irq, irq_handler_t handler, void *dev);
void dispatch_irq(int irq);

// typedef void (*isrptr_t)(int vector, struct interrupt_frame* frame);
// void isrHandler(int index, struct interrupt_frame* frame);
// void register_interrupt_handler(int vector, isrptr_t handler);
// void init_timer(uint32_t frequency);
// void gp_fault_handler(int vector, struct interrupt_frame* frame);


