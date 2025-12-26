#include "stddef.h"
#include "stdint.h"
#include "idt.h"
#include "util.h"
#include "isr.h"
#include "kprint.h"

#define IDT_DESCRIPTORS 256

extern void             idt_load(struct idt_metadata *idt_metadata);
extern void*            isr_pointer_table[]; // 256 pointers
extern void             init_timer(uint32_t hz);
extern void             init_keyboard(void);

static IdtDescriptor  idt_table[IDT_DESCRIPTORS];

static void set_idt_entry(int intex, uintptr_t *isr, uint8_t flags, uint8_t ist)
{
        uint64_t addr = (uint64_t)isr;

        idt_table[intex].offset_low  = addr & 0xFFFF;
        idt_table[intex].selector    = 0x08;     // kernel code segment selector
        idt_table[intex].ist         = ist & 0x7;
        idt_table[intex].type_attr   = flags;    // e.g. 0x8E = present, ring0, 64-bit interrupt gate
        idt_table[intex].offset_mid  = (addr >> 16) & 0xFFFF;
        idt_table[intex].offset_high = (addr >> 32) & 0xFFFFFFFF;
        idt_table[intex].reserved    = 0;
}

/* Set up CPU exceptions (0–31) */ 
void idt_init(void)
{
        // Fill exception handlers (0–31)
        for (int i = 0; i < 32; i++)
                set_idt_entry(i, isr_pointer_table[i], 0x8E, 0);

        // Load IDT
        IdtMetadata idt_metadata;
        idt_metadata.limit = sizeof(idt_table) - 1;
        idt_metadata.base  = (uint64_t)&idt_table;
        idt_load(&idt_metadata);

        register_interrupt_handler(13, gp_fault_handler);   // GP fault

        // // Timer IRQ (intex 32)
        // set_idt_entry(32, isr_pointer_table[32], 0x8E, 0);
        // init_timer(100);
        // outb(PIC1_DATA, 0xFE); // Unmask the timer (since at IRQ = 0)

        // Keyboard IRQ (intex 33)
        set_idt_entry(33, isr_pointer_table[33], 0x8E, 0);
        init_keyboard();
        outb(PIC1_DATA, 0xFC); // Unmask keyboard interrupt

        __asm__ volatile ("sti"); // Unmask all interrupts
}
