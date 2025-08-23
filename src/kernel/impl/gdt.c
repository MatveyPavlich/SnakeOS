#include "stddef.h"
#include "stdint.h"
#include "gdt.h"
#define GDT_SEGMENT_DESCRIPTOR_COUNT 5
#define GDT_SYSTEM_DESCRIPTOR_COUNT 1

extern void loadGdtr(GdtMetadata *m);

void gdtInit() {
    static GdtSegmentDescriptor gdt_segment_entries[GDT_SEGMENT_DESCRIPTOR_COUNT] __attribute__((aligned(8)));
    // static GdtSystemDescriptor  gdt_system_entries[GDT_SYSTEM_DESCRIPTOR_COUNT]   __attribute__((aligned(8)));
 
    gdt_segment_entries[0] = createGdtSegmentDescriptor(0, 0,       0,    0   ); // Null descriptor
    gdt_segment_entries[1] = createGdtSegmentDescriptor(0, 0xFFFFF, 0x9A, 0x20); // Kernel code segment
    gdt_segment_entries[2] = createGdtSegmentDescriptor(0, 0xFFFFF, 0x92, 0xC0); // Kernel data segment
    gdt_segment_entries[3] = createGdtSegmentDescriptor(0, 0xFFFFF, 0xFA, 0x20); // User code segment
    gdt_segment_entries[4] = createGdtSegmentDescriptor(0, 0xFFFFF, 0xF2, 0xC0); // User data segment
    // gdt_segment_entries[5] = createGdtSystemDescriptor(5, , , ); // TSS

    GdtMetadata gdt_metadata;
    gdt_metadata.gdt_pointer = (uintptr_t)gdt_segment_entries;
    gdt_metadata.gdt_size = (sizeof(GdtSegmentDescriptor) * GDT_SEGMENT_DESCRIPTOR_COUNT - 1); // TODO: adjust GDT metadata to include system descriptors

    loadGdtr(&gdt_metadata);

    return;
}

GdtSystemDescriptor createGdtSystemDescriptor(uint64_t base, uint32_t limit) {
    GdtSystemDescriptor d;

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

GdtSegmentDescriptor createGdtSegmentDescriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    /*
    limit = 20 bits; base = 32 bits; access = 8 btis; flags = 4 bits
    */
    if(limit > 0xFFFFF)
        GdtError();
    GdtSegmentDescriptor d = {0};

    d.low_limit = (uint16_t)(limit & 0xFFFFu);
    d.low_base  = (uint16_t)(base & 0xFFFFu);
    d.mid_base  = (uint8_t)((base >> 16) & 0xFFu);
    d.access_bytes = access;
    d.flags_and_high_limit =
        (uint8_t)((flags & 0xF0u) | ((limit >> 16) & 0x0Fu));
    d.high_base = (uint8_t)((base >> 24) & 0xFFu);

    return d;
}
