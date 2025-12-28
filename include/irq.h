// API interface for irq module
#include "interrupt.h"

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

typedef void (*irq_handler_t)(int irq, struct interrupt_frame *, void *dev_id);

/*
 * irq_handle - Route interrupt to a relevant handler function.
 * @irq:        IRQ for the interrupt to handle. 
 * @frame:      CPU state before receiving the interrupt.
 */
void irq_handle(int irq, struct interrupt_frame* frame);

/*
 * irq_request - Add a handler to an interrupt line.
 * @irq:         The interrupt line to allocate.
 * @handler:     Function to call when IRQ occurs.
 * @dev:	 A cookie passed to the handler function.
 * Caller must check if the allocation was successful.
 */
int irq_request(int irq, irq_handler_t handler, void *dev);
