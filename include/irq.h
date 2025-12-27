// API interface for irq module
#include "stdint.h"

/* irq_chip - structure to represent all properties of an IRQ chip
 * @name:            Name of the chip
 * @irq_init:        Int the chip
 * @irq_shutdown:    Mask all interrupts to prevent them going into the buffer.
 *                   Do this before switching to a different irq_chip
 * @irq_mask:        Mask a specific irq.
 * @irq_unmask:      Unmask a specific irq.
 * @irq_eoi:         Send an End Of Interrupt (EOI) signal to irq_chip
 */
struct irq_chip {
        const char *name;
        void (*irq_init)(void);
        void (*irq_shutdown)(void);
        void (*irq_mask)(int irq);
        void (*irq_unmask)(int irq);
        void (*irq_eoi)(int irq);
};

int irq_set_chip(struct irq_chip *chip); /* Register interrupt controller with
                                             the IRQ core */

/* interrupt_frame - Structure to capture the CPU state before the interrupt
 * entry
 */
struct interrupt_frame {
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t error_code;
};

typedef void (*irq_handler_t)(int irq, struct interrupt_frame *, void *dev_id);
void irq_handle(int irq, struct interrupt_frame* frame);
int irq_request(int irq, irq_handler_t handler, void *dev);
