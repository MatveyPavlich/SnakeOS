// Logic adopted from linux kernel include/linux/interrupt.h
// API implementation of the irq module

#include "irq.h"
#include "kprintf.h"

#define NMBR_IRQS 16

struct irq_desc {
        irq_handler_t handler;
        void *dev_id;
};

static struct irq_desc irq_table[NMBR_IRQS];
static struct irq_chip *irq_chip_active;

/*
 * irq_handle - Route interrupt to a relevant handler function.
 * @irq:        IRQ for the interrupt to handle. 
 * @frame:      CPU state before receiving the interrupt.
 */
void irq_handle(int irq, struct interrupt_frame* frame)
{
        if (irq < 0 || irq >= NMBR_IRQS) {
                kprintf("ERROR: invalid IRQ (%d) in irq_handler.\n", irq);
                kprintf("System halted.\n");
                while (1) __asm__("hlt");
        }

        struct irq_desc *desc = &irq_table[irq];
        if (desc->handler)
                desc->handler(irq, frame, desc->dev_id);
        else
                kprintf("Unhandled IRQ: %d. No action taken.\n", irq);

        if (irq_chip_active && irq_chip_active->irq_eoi)
                irq_chip_active->irq_eoi(irq);
}

int irq_set_chip(struct irq_chip *chip)
{
        if (!chip) {
                kprintf("ERROR: irq_set_chip(NULL)\n");
                return 1;
        }
	irq_chip_active = chip;

	if (irq_chip_active->irq_init) {
                irq_chip_active->irq_init();
                return 0;
        } else {
                kprintf("ERROR: irq_chip is missing irq_init() method\n");
                return 1;
        }
}

/*
 * irq_request - Add a handler to an interrupt line.
 * @irq:         The interrupt line to allocate.
 * @handler:     Function to call when IRQ occurs.
 * @dev:	 A cookie passed to the handler function.
 * Caller must check if the allocation was successful.
 */
int irq_request(int irq, irq_handler_t handler, void *dev)
{
        if (irq < 0 || irq >= NMBR_IRQS) {
                kprintf("ERROR: %d is an invalid IRQ number\n", irq);
                return 1;
        }
        if (!(irq_chip_active) || !(irq_chip_active->irq_unmask)) {
                kprintf("ERROR: irq_chip inactive or has no irq_unmask method\n");
                return 1;
        }

        irq_table[irq] = (struct irq_desc) {
                .handler = handler,
                .dev_id  = dev,
        };
        irq_chip_active->irq_unmask(irq);

        return 0;
}
