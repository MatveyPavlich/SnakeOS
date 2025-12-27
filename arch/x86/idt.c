/* Implementation of the 64-bit idt setup */

#include "stdint.h"
#include "isr.h"

#define IDT_DESCRIPTORS 256
#define KERNEL_CS       0x08 // kernel code segment selector

extern void             idt_load(struct idt_metadata *idt_metadata);
extern void*            idt_stub_table[]; // 256 pointers

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

static struct idt_descriptor_64 idt_table[IDT_DESCRIPTORS];

/* set_idt_entry - Helper to fill the idt_descriptor with relevant flags.
 * @index:         Vector of the IDT desriptor in the table.
 * @isr:           Pointer to the IRS that will handle the interrupt.
 * @flags:         Flags for the IDT descriptor.
 * @ist:           Interrupt Stack Table (ist) pointer to switch to the correct
 *                 stack to deal with the interrupt.
 */
static void set_idt_entry(int index, uint_64_t isr_addr, uint8_t flags,
                          uint8_t ist)
{
        idt_table[index] = (struct idt_descriptor_64) {
                .offset_low  = isr_addr & 0xFFFF,
                .selector    = KERNEL_CS,
                .ist         = ist & 0x7,
                .type_attr   = flags, // e.g. 0x8E = present, ring0, 64-bit
                                      // interrupt gate
                .offset_mid  = (isr_addr >> 16) & 0xFFFF,
                .offset_high = (isr_addr >> 32) & 0xFFFFFFFF,
                .reserved    = 0,
        }
}

/* Set up CPU exceptions (0–31) */ 
void idt_init(void)
{
        // Fill exception handlers (0–31)
        for (int i = 0; i < 32; i++)
                set_idt_entry(i, idt_stub_table[i], 0x8E, 0);

        struct idt_metadata idt_metadata = {
                .limit = sizeof(idt_table) - 1,
                .base  = (uint64_t)&idt_table,
        }
        idt_load(&idt_metadata); /* Load usign lidt instruction */

        register_interrupt_handler(13, gp_fault_handler);   // GP fault

}
// // Timer IRQ (intex 32)
// set_idt_entry(32, isr_pointer_table[32], 0x8E, 0);
// init_timer(100);
// outb(PIC1_DATA, 0xFE); // Unmask the timer (since at IRQ = 0)

// Keyboard IRQ (intex 33)
// set_idt_entry(33, isr_pointer_table[33], 0x8E, 0);
// init_keyboard();
// outb(PIC1_DATA, 0xFC); // Unmask keyboard interrupt

// __asm__ volatile ("sti"); // Unmask all interrupts
