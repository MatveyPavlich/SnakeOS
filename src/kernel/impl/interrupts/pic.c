#include "stdint.h"
#include "util.h"

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1

#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01

void pic_init(void) {
    // Start initialization sequence (cascade mode, expect ICW4)
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);

    // Remap offsets: master to 0x20 (32), slave to 0x28 (40)
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Setup cascading
    outb(PIC1_DATA, 0x04); // Tell master that slave is at IRQ2
    outb(PIC2_DATA, 0x02); // Tell slave its cascade identity

    // Environment info
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Mask all IRQs except IRQ0 (timer)
    outb(PIC1_DATA, 0xFE);
    outb(PIC2_DATA, 0xFF);
}
