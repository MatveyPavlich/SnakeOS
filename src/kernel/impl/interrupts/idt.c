#include "stddef.h"
#include "stdint.h"
#include "idt.h"

#define IDT_DESCRIPTORS 256

extern void *isr_stub_table[];   // generate array of asm stubs
extern void loadIdt(IdtMetadata *idt_metadata);

static IdtDescriptor idt[IDT_DESCRIPTORS];

static void setIdtEntry(int intex, void *isr, uint8_t flags, uint8_t ist) {
    uint64_t addr = (uint64_t)isr;

    idt[intex].offset_low  = addr & 0xFFFF;
    idt[intex].selector    = 0x08;     // kernel code segment selector
    idt[intex].ist         = ist & 0x7;
    idt[intex].type_attr   = flags;    // e.g. 0x8E = present, ring0, 64-bit interrupt gate
    idt[intex].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[intex].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[intex].reserved    = 0;
}

void idtInit(void) {
    // Fill exception handlers (0–31)
    for (int i = 0; i < 32; i++) {
        setIdtEntry(i, isr_stub_table[i], 0x8E, 0); // ring0, interrupt gate
    }

    // Example: Timer IRQ (intex 32)
    setIdtEntry(32, isr_stub_table[32], 0x8E, 0);

    // Example: Keyboard IRQ (intex 33)
    setIdtEntry(33, isr_stub_table[33], 0x8E, 0);

    // Load IDT
    IdtMetadata idt_metadata;
    idt_metadata.limit = sizeof(idt) - 1;
    idt_metadata.base  = (uint64_t)&idt;
    loadIdt(&idt_metadata);
}
