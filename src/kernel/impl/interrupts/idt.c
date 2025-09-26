#include "stddef.h"
#include "stdint.h"
#include "idt.h"
#include "util.h"
#include "isr.h"
#include "kprint.h"

#define IDT_DESCRIPTORS 256
#define PIC1_CMD        0x20
#define PIC1_DATA       0x21
#define PIC2_CMD        0xA0
#define PIC2_DATA       0xA1

extern void             loadIdt(IdtMetadata *idt_metadata);
extern void*            isr_pointer_table[]; // 256 pointers
extern void             initTimer(uint32_t hz);
extern void             init_keyboard(void);

static IdtDescriptor  idt_table[IDT_DESCRIPTORS];



static void setIdtEntry(int intex, uintptr_t *isr, uint8_t flags, uint8_t ist) {
    uint64_t addr = (uint64_t)isr;

    idt_table[intex].offset_low  = addr & 0xFFFF;
    idt_table[intex].selector    = 0x08;     // kernel code segment selector
    idt_table[intex].ist         = ist & 0x7;
    idt_table[intex].type_attr   = flags;    // e.g. 0x8E = present, ring0, 64-bit interrupt gate
    idt_table[intex].offset_mid  = (addr >> 16) & 0xFFFF;
    idt_table[intex].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt_table[intex].reserved    = 0;
}

void idtInit(void) {

    /*================ Set up CPU exceptions (0–31) ================*/

    // Fill exception handlers (0–31)
    for (int i = 0; i < 32; i++) {
        setIdtEntry(i, isr_pointer_table[i], 0x8E, 0); // ring0, interrupt gate
    }

    // Load IDT
    IdtMetadata idt_metadata;
    idt_metadata.limit = sizeof(idt_table) - 1;
    idt_metadata.base  = (uint64_t)&idt_table;
    loadIdt(&idt_metadata);

    register_interrupt_handler(13, gp_fault_handler);   // GP fault

    /*================ Set up PIC to enable hardware interrupts ================*/
    // Start initialization sequence (cascade mode, expect ICW4)
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    // Map master to 0x20-0x27 (32-39) IDT entries, slave to 0x28-2F (40-47) IDT entries
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Setup cascading
    outb(PIC1_DATA, 0x04); // Tell master that slave is at IRQ2
    outb(PIC2_DATA, 0x02); // Tell slave its cascade identity

    // Environment info
    outb(PIC1_DATA, 0x1);
    outb(PIC2_DATA, 0x1);

    // Mask all IRQs until they are set up (to make sure they don't go into the buffer)
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    // Example: Timer IRQ (intex 32)
    setIdtEntry(32, isr_pointer_table[32], 0x8E, 0);
    initTimer(100);
    outb(PIC1_DATA, 0xFE); // Unmask the timer (since at IRQ = 0)

    // // Example: Keyboard IRQ (intex 33)
    setIdtEntry(33, isr_pointer_table[33], 0x8E, 0);
    init_keyboard();
    outb(PIC1_DATA, 0xFC); // Unmask keyboard interrupt

    __asm__ volatile ("sti"); // Unmask all interrupts
}
