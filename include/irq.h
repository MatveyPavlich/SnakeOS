// API interface for irq module
#include "stdint.h"

/* irq_chip - structure to represent all properties of an IRQ chip
 * @name:            name of the chip
 * @irq_init:        int the chip
 * @irq_shutdown:    mask all interrupts to prevent them going into the buffer.
 *                   Do this before switching to a different irq_chip
 * @irq_mask_irq:    mask a specific irq
 * @irq_unmask_irq:  unmask a specific irq
 * @irq_eoi:         send an End Of Interrupt (EOI) signal to irq_chip
 */
struct irq_chip {
        const char *name;
        void (*irq_init)(void);
        void (*irq_shutdown)(void);
        void (*irq_mask_irq)(int irq);
        void (*irq_unmask_irq)(int irq);
        void (*irq_eoi)(int irq);
};

void irq_set_chip(struct irq_chip *chip); /* Register interrupt controller with
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

typedef void (*irq_handler_t)(int irq, struct interrupt_frame *);
int irq_request(int irq, irq_handler_t handler, void *dev);
void irq_dispatch(int irq);
