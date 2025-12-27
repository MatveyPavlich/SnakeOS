/* Impelentation of the i8259A controller (PIC) */

#include "util.h"
#include "irq.h"

#define PIC1         0x20       /* IO port base address for master PIC */
#define PIC2         0xA0       /* IO port base address for slave PIC */
#define PIC1_CMD     PIC1
#define PIC1_DATA    (PIC1 + 1)
#define PIC2_CMD     PIC2
#define PIC2_DATA    (PIC2 + 1)
#define PIC1_IDT_POS 0x20       /* Map master to 32-39 IDT entries */
#define PIC2_IDT_POS 0x28       /* Map master to 40-47 IDT entries */
#define PIC_EOI      0x20

/* Forward declarations */
static void i8259A_init(void);
static void i8259A_mask_all(void);
static void i8259A_mask_irq(int irq);
static void i8259A_unmask_irq(int irq);
static void i8259A_send_eoi(int irq);

static struct irq_chip i8259A_chip = {
	.name            = "i8259A",
	.irq_init        = i8259A_init,
	.irq_shutdown    = i8259A_mask_all,
	.irq_mask        = i8259A_mask_irq,
        .irq_unmask      = i8259A_unmask_irq,
	.irq_eoi         = i8259A_send_eoi,
};

/* Init hook to be exposed for the registration */
void irq_arch_init(void)
{
	irq_set_chip(&i8259A_chip);
}

/* Set up PIC to enable hardware interrupts */
static void i8259A_init(void)
{

        /* Start initialization sequence (cascade mode, expect ICW4) */
        outb(PIC1_CMD, 0x11);
        outb(PIC2_CMD, 0x11);

        /* Map master & slave to relevant IDT entries */
        outb(PIC1_DATA, PIC1_IDT_POS);
        outb(PIC2_DATA, PIC2_IDT_POS);

        /* Setup cascading */
        outb(PIC1_DATA, 0x04); // Tell master that slave is at IRQ2
        outb(PIC2_DATA, 0x02); // Tell slave its cascade identity

        /* Environment info */
        outb(PIC1_DATA, 0x1);
        outb(PIC2_DATA, 0x1);

        /* Mask all IRQs. Useful for making sure they don't go into the buffer */
        i8259A_mask_all();

}

/* Mask all IRQs. Useful for making sure they don't go into the buffer */
static void i8259A_mask_all(void)
{
        outb(PIC1_DATA, 0xFF);
        outb(PIC2_DATA, 0xFF);
}

/* i8259A_mask_irq - mask specified interrupt line to deactivate it
 * @irq:             Interrupt request line
 */
static void i8259A_mask_irq(int irq)
{
	uint16_t port;
	uint8_t value;

	if (irq < 8) port = PIC1_DATA;
	else {
		port = PIC2_DATA;
		irq -= 8; // slave IRQs are 0–7 internally
	}

	value = inb(port);
	value |= (1 << irq); // set bit to mask

	outb(port, value);
}

/* i8259A_unmask_irq - unmask specified interrupt line to activate it
 * @irq:               Interrupt request line
 */
static void i8259A_unmask_irq(int irq)
{
        uint16_t port;
        uint8_t value;

        // Master PIC
        if (irq < 8) port = PIC1_DATA;
        // Slave PIC
        else {
                port = PIC2_DATA;
                irq -= 8; // slave IRQs are 0–7 internally
        }

        value = inb(port);
        value &= ~(1 << irq); // clear bit to unmask
        outb(port, value);
}

/* i8259A_send_eoi - send End Of Interrupt (EOI) signal to PIC to inform that
 *                   we can start handling the next interrupt
 * @irq:             Interrupt request line
 * Always send EOI to master. However, if the IRQ is on slave then send EOI to
 * it first, then master.
 */
static void i8259A_send_eoi(int irq)
{
        // Slave PIC
        if (irq >= 8) outb(PIC2_CMD, PIC_EOI);

        // Master PIC (always)
        outb(PIC1_CMD, PIC_EOI);
}
