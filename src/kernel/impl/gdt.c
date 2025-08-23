#include "stddef.h"
#include "stdint.h"
#include "gdt.h"
#define GDT_SIZE 5

extern void loadGdtr(GdtMetadata *m);

void gdtInit() {
    static SegmentDescriptor gdt_entries[GDT_SIZE] __attribute__((aligned(8)));

    gdt_entries[0] = createSegmentDescriptor(0, 0,       0,    0   ); // Null descriptor
    gdt_entries[1] = createSegmentDescriptor(0, 0xFFFFF, 0x9A, 0x20); // Kernel code segment
    gdt_entries[2] = createSegmentDescriptor(0, 0xFFFFF, 0x92, 0xC0); // Kernel data segment
    gdt_entries[3] = createSegmentDescriptor(0, 0xFFFFF, 0xFA, 0x20); // User code segment
    gdt_entries[4] = createSegmentDescriptor(0, 0xFFFFF, 0xF2, 0xC0); // User data segment
    // gdt_entries[5] = createSystemDescriptor(5, , , ); // TSS

    GdtMetadata gdt_metadata;
    gdt_metadata.gdt_pointer = (uintptr_t)gdt_entries;
    gdt_metadata.gdt_size = (sizeof(SegmentDescriptor) * GDT_SIZE - 1);

    loadGdtr(&gdt_metadata);

    return;
}

SystemDescriptor createSystemDescriptor(uint64_t base, uint32_t limit) {
    SystemDescriptor d;

    d.limit_low  = limit & 0xFFFF;
    d.base_low   = base & 0xFFFF;
    d.base_mid   = (base >> 16) & 0xFF;
    d.access     = 0x89;   // 10001001b = present, type=available 64-bit TSS
                              // use 0x89 (available) or 0x89 | 0x2 = 0x8B (busy)
    d.gran       = ((limit >> 16) & 0x0F);
    d.gran      |= 0x00;   // G=0, AVL=0, L=0, DB=0 (must be zero for TSS)
    d.base_high  = (base >> 24) & 0xFF;

    d.base_upper = (base >> 32) & 0xFFFFFFFF;
    d.reserved   = 0;

    return d;
}


void GdtError() {
    while (1) __asm__("hlt");
}

SegmentDescriptor createSegmentDescriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    /*
    limit = 20 bits; base = 32 bits; access = 8 btis; flags = 4 bits
    */
    if(limit > 0xFFFFF)
        GdtError();
    SegmentDescriptor d = {0};

    d.low_limit = (uint16_t)(limit & 0xFFFFu);
    d.low_base  = (uint16_t)(base & 0xFFFFu);
    d.mid_base  = (uint8_t)((base >> 16) & 0xFFu);
    d.access_bytes = access;
    d.flags_and_high_limit =
        (uint8_t)((flags & 0xF0u) | ((limit >> 16) & 0x0Fu));
    d.high_base = (uint8_t)((base >> 24) & 0xFFu);

    return d;
}
