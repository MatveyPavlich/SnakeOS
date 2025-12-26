/* i8259 controller set up (PIC) */

#include "util.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

/* Set up PIC to enable hardware interrupts */
void pic_init(void)
{

        // Start initialization sequence (cascade mode, expect ICW4)
        outb(PIC1_CMD, 0x11);
        outb(PIC2_CMD, 0x11);

        /* Map master to 0x20-0x27 (32-39) IDT entries, slave to 0x28-2F (40-47)
         * IDT entries
         */
        outb(PIC1_DATA, 0x20);
        outb(PIC2_DATA, 0x28);

        // Setup cascading
        outb(PIC1_DATA, 0x04); // Tell master that slave is at IRQ2
        outb(PIC2_DATA, 0x02); // Tell slave its cascade identity

        // Environment info
        outb(PIC1_DATA, 0x1);
        outb(PIC2_DATA, 0x1);

        /* Mask all IRQs. Useful for making sure they don't go into the buffer */
        outb(PIC1_DATA, 0xFF);
        outb(PIC2_DATA, 0xFF);
}

/* pic_unmask_irq - unmask specified interrupt line to activate it
 * @irq:            Interrupt request line
void pic_unmask_irq(int irq)
{
        uint16_t port;
        uint8_t value;

        // Master PIC
        if (irq < 8) port = PIC1_DATA;
        // Slave PIC
        else {
                port = PIC2_DATA;
                irq -= 8;   // slave IRQs are 0–7 internally
        }

        value = inb(port);
        value &= ~(1 << irq); // clear bit to unmask
        outb(port, value);
}

/* pic_send_eoi - send signal to slave for the interrupt
 * @irq:          Interrupt request line
 * If the request is for the interrupt line on the slave then save the request
 * to it first and then mater. Else, just master
 */
void pic_send_eoi(int irq)
{
        // Slave PIC
        if (irq >= 8) outb(PIC2_CMD, 0x20);

        // Master PIC (always)
        outb(PIC1_CMD, 0x20);
}
