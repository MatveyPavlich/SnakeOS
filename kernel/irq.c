// Logic adopted from linux kernel include/linux/interrupt.h
// API implementation of the irq module

#include "interrupt.h"
#include "irq_chip.h"
#include "kprint.h"

#define NMBR_IRQS 16

struct irq_desc {
        irq_handler_t handler;
        void *dev_id;
};

static struct irq_desc irq_table[NMBR_IRQS];
static struct irq_chip *irq_chip_active;

/* irq_handle - Route an interrupt to the handler function. To be used only
 *              in the CPU stuff (e.g., IDT) to direct interrupts into the irq
 *              subsystem. Do not exposed to drivers or callers.
 * @irq:        IRQ for the interrupt to handle. 
 * @frame:      CPU state before receiving the interrupt.
 */
void irq_handle(int irq, struct interrupt_frame* frame)
{
        if (irq < 0 || irq >= NMBR_IRQS) {
                kprint("ERROR: invalid IRQ (%d) in irq_handler.\n", irq);
                kprint("System halted.\n");
                while (1) __asm__("hlt");
        }

        struct irq_desc *desc = &irq_table[irq];
        if (desc->handler)
                desc->handler(irq, frame, desc->dev_id);
        else
                kprint("Unhandled IRQ: %d. No action taken.\n", irq);

        if (irq_chip_active && irq_chip_active->irq_eoi)
                irq_chip_active->irq_eoi(irq);
}

int irq_set_chip(struct irq_chip *chip)
{
        if (!chip) {
                kprint("ERROR: irq_set_chip(NULL)\n");
                return 1;
        }
	irq_chip_active = chip;

	if (irq_chip_active->irq_init) {
                irq_chip_active->irq_init();
                return 0;
        } else {
                kprint("ERROR: irq_chip is missing irq_init() method\n");
                return 1;
        }
}

int irq_request(int irq, irq_handler_t handler, void *dev)
{
        kprint("Requesting %d IRQ\n", irq);
        if (irq < 0 || irq >= NMBR_IRQS) {
                kprint("ERROR: %d is an invalid IRQ number\n", irq);
                return 1;
        }
        if (!(irq_chip_active) || !(irq_chip_active->irq_unmask)) {
                kprint("ERROR: irq_chip inactive or has no irq_unmask method\n");
                return 1;
        }

        irq_table[irq] = (struct irq_desc) {
                .handler = handler,
                .dev_id  = dev,
        };
        irq_chip_active->irq_unmask(irq);

        return 0;
}
