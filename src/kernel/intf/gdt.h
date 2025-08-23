#include "stddef.h"
#include "stdint.h"

// GdtSegmentDescriptor is a GDT table row that describe a segments (8 bytes)
typedef struct {
    uint16_t low_limit;            // limit[15:0]
    uint16_t low_base;             // base[15:0]
    uint8_t  mid_base;             // base[23:16]
    uint8_t  access_bytes;         // P|DPL|S|Type
    uint8_t  flags_and_high_limit; // Flags(high nibble) | limit[19:16](low nibble)
    uint8_t  high_base;            // base[31:24]

} __attribute__((packed)) GdtSegmentDescriptor;

// GdtSystemDescriptor is a GDT table row that describe a system segment (16 bytes)
// Note that they are extended to 16 bytes in long mode => equvalent to x2 segment descriptors
typedef struct {
    uint16_t limit_low;            // Limit 0:15
    uint16_t base_low;             // Base 0:15
    uint8_t  base_mid;             // Base 16:23
    uint8_t  access;               // Type(0x9 / 0xB) | P=1 | DPL | S=0
    uint8_t  gran;                 // Limit 16:19 | flags
    uint8_t  base_high;            // Base 24:31

    // extra 8 bytes for 64-bit TSS
    uint32_t base_upper;           // Base 32:63
    uint32_t reserved;             // Must be 0
} __attribute__((packed)) GdtSystemDescriptor;


// TSS entry
typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;      // rsp when entering ring 0
    uint64_t rsp1;      // rsp when entering ring 1
    uint64_t rsp2;      // rsp when entering ring 2
    uint64_t reserved1;
    // The next 7 entries are the "Interrupt stack Table"
    // Here we can define stack pointers to use when handling interrupts.
    // Which one to use is defined in the Interrupt Descriptor Table.
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap;

} __attribute__((packed)) Tss64Entry;


typedef struct {
    uint16_t  gdt_size;
    uintptr_t gdt_pointer;

} __attribute__((packed)) GdtMetadata;

void gdtInit();
GdtSegmentDescriptor createGdtSegmentDescriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
GdtSystemDescriptor createGdtSystemDescriptor(uint64_t base, uint32_t limit);