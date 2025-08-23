#include "stddef.h"
#include "stdint.h"
#include "idt.h"

extern void *isr_stub_table[];   // array of asm stubs
extern void loadIdt(IdtDescriptor *idtr);

static IdtEntry idt[IDT_SIZE];

static void setIdtEntry(int vector, void *isr, uint8_t flags, uint8_t ist) {
    uint64_t addr = (uint64_t)isr;

    idt[vector].offset_low  = addr & 0xFFFF;
    idt[vector].selector    = 0x08;     // kernel code segment selector
    idt[vector].ist         = ist & 0x7;
    idt[vector].type_attr   = flags;    // e.g. 0x8E = present, ring0, 64-bit interrupt gate
    idt[vector].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[vector].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero        = 0;
}

void idtInit(void) {
    // Fill exception handlers (0–31)
    for (int i = 0; i < 32; i++) {
        setIdtEntry(i, isr_stub_table[i], 0x8E, 0); // ring0, interrupt gate
    }

    // Example: Timer IRQ (vector 32)
    setIdtEntry(32, isr_stub_table[32], 0x8E, 0);

    // Example: Keyboard IRQ (vector 33)
    setIdtEntry(33, isr_stub_table[33], 0x8E, 0);

    // Load IDT
    IdtDescriptor idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;
    loadIdt(&idtr);
}
