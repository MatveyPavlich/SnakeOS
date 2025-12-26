// Logic adopted from linux kernel include/linux/interrupt.h
// API implementation of the irq module

#define IRQ_NMBR 256

struct irq_desc {
        irq_handler_t handler;
        void *dev_id;
};

static struct irq_desc irq_table[IRQ_NMBR];

/*
 * request_irq - Add a handler to an interrupt line
 * @irq:         Then interrupt line to allocate
 * @handler:     Function to call when IRQ occurs
 * @dev:	 A cookie passed to the handler function
 */
int register_irq(int irq, irq_handler_t handler, void *dev)
{
        if (irq < 0 || irq >= IRQ_NMBR) {
                kprint("ERROR: invalid IRQ\n");
                return 1;
        }

        irq_table[irq].handler = handler;
        irq_table[irq].dev_id = dev;
        return 0;
}

/*
 * dispatch_irq - Add a handler to an interrupt line
 * @iqr:         Then interrupt line to allocate
 */
void dispatch_irq(int irq)
{
        if (irq_table[irq].handler)
                irq_table[irq].handler(irq, irq_table[irq].dev_id);
}
