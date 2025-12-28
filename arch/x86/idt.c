/* Implementation of the 64-bit idt setup */

#include "stdint.h"

#define IDT_DESCRIPTORS         256
#define KERNEL_CS               0x08 // kernel code segment selector
#define IDT_FLAG_INTERRUPT_GATE 0x8E


/* Format of the 64-bit idt table row (i.e., descriptor) for an interrupt */
struct idt_descriptor_64 {
        uint16_t offset_low;   // bits 0..15 of handler function address
        uint16_t selector;     // code segment selector in GDT
        uint8_t  ist;          // bits 0..2 = IST index, rest zero
        uint8_t  type_attr;    // gate type, DPL, P
        uint16_t offset_mid;   // bits 16..31 of handler address
        uint32_t offset_high;  // bits 32..63 of handler address
        uint32_t reserved;     // do not touch, some intel magic
} __attribute__((packed));

/* Metadata about the IDT to be loaded into the CPU with lidt instruction */
struct idt_metadata {
        uint16_t limit;
        uint64_t base;
} __attribute__((packed));

/* Temp fix: make sure idt_load() is after idt_metadata struct declaration */
extern void idt_load(struct idt_metadata *idt_metadata);
extern void* idt_stub_table[]; // 256 pointers
static struct idt_descriptor_64 idt_table[IDT_DESCRIPTORS];

/* set_idt_entry - Helper to fill the idt_descriptor with relevant flags.
 * @index:         Vector of the IDT desriptor in the table.
 * @isr_addr:      Pointer to the ISR that will handle the interrupt.
 * @flags:         Flags for the IDT descriptor.
 * @ist:           Interrupt Stack Table (ist) pointer to switch to the correct
 *                 stack to deal with the interrupt.
 */
static void set_idt_entry(int index, uint64_t isr_addr, uint8_t flags,
                          uint8_t ist)
{
        idt_table[index] = (struct idt_descriptor_64) {
                .offset_low  = isr_addr & 0xFFFF,
                .selector    = KERNEL_CS,
                .ist         = ist & 0x7,
                .type_attr   = flags, /* e.g. 0x8E = present, ring0, 64-bit
                                         interrupt gate */
                .offset_mid  = (isr_addr >> 16) & 0xFFFF,
                .offset_high = (isr_addr >> 32) & 0xFFFFFFFF,
                .reserved    = 0,
        };
}

/* Fill IDT with stubs that will direct interrupts into irq subsystem
 * through irq_handle_vector() in its API
 */
void idt_init(void)
{
        /* Fill every interrupt with a valid stub to fail gracefully */
        for (int i = 0; i < IDT_DESCRIPTORS; i++)
                set_idt_entry(i, (uint64_t)idt_stub_table[i], IDT_FLAG_INTERRUPT_GATE, 0);

        /* CPU copies the contents of idt_metadata, so we don't need to store
         * the struct anywhere
         */
        struct idt_metadata idt_metadata = {
                .limit = sizeof(idt_table) - 1,
                .base  = (uint64_t)&idt_table,
        };

        idt_load(&idt_metadata); /* Load IDT into the CPU using the lidt
                                    instruction */
}
