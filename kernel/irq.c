// Logic adopted from linux kernel include/linux/interrupt.h
// API implementation of the irq module

#include "irq.h"
#include "kprint.h"

#define IRQ_NMBR 256

struct irq_desc {
        irq_handler_t handler;
        void *dev_id;
};

static struct irq_desc irq_table[IRQ_NMBR];
static struct irq_chip *irq_chip_active;

/* Called by all IDT entries (index & frame are pushed by the asm stub) */
void irq_handle_vector(int index, struct interrupt_frame* frame)
{
        struct irq_desc *desc = &irq_table[index];

        if (desc->handler)
                desc->handler(index, frame);
        else {
                kprintf("Unhandled interrupt: %d\n", index);
                kprintf("System halted.\n");
                while (1) __asm__("hlt");
        }

        irq_chip_active->irq_eoi(irq);
}

void irq_set_chip(struct irq_chip *chip)
{
	irq_chip_active = chip;

	if (irq_chip_active->irq_init)
                irq_chip_active->irq_init();
}

/*
 * irq_request - Add a handler to an interrupt line
 * @irq:         Then interrupt line to allocate
 * @handler:     Function to call when IRQ occurs
 * @dev:	 A cookie passed to the handler function
 * Caller must check if the allocation was successful.
 */
int irq_request(int irq, irq_handler_t handler, void *dev)
{
        if (irq < 0 || irq >= IRQ_NMBR)
                return 1;

        irq_table[irq] = (struct irq_desc) {
                .handler = handler,
                .dev_id  = dev,
        };

        return 0;
}

/*
 * irq_dispatch - Add a handler to an interrupt line
 * @iqr:          The interrupt line to allocate
 * Caller must check if the dispatch was successful
 */
int irq_dispatch(int irq, struct interrupt_frame *frame)
{
        struct irq_desc *desc = &irq_table[index];
        if (desc->handler)
                desc->handler(irq, frame);
        else
                return 1;

        irq_chip_active->irq_eoi(irq);
        if (irq_chip_active->irq_eoi)
                irq_chip_active->irq_eoi(irq);

        return 0;
}

// TODO: Implement here for now, but later should be in exceptions.c
static void gp_fault_handler(int vector, struct interrupt_frame* frame)
{
        kprintf("General Protection Fault: %d\n", vector);
        kprintf("RIP = %x\n",frame->rip);
        kprintf("System halted\n");

        while (1) __asm__("hlt");
}
