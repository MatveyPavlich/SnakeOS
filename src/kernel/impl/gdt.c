#include "stddef.h"
#include "stdint.h"
#include "gdt.h"

extern void loadGdtr(GdtMetadata *m);

void gdtInit() {
    static GdtDescriptor gdt_entries[5] __attribute__((aligned(8)));

    gdt_entries[0] = createGdtDescriptor(0, 0,       0,    0   ); // Null descriptor
    gdt_entries[1] = createGdtDescriptor(0, 0xFFFFF, 0x9A, 0x20); // Kernel code segment
    gdt_entries[2] = createGdtDescriptor(0, 0xFFFFF, 0x92, 0xC0); // Kernel data segment
    gdt_entries[3] = createGdtDescriptor(0, 0xFFFFF, 0xFA, 0x20); // User code segment
    gdt_entries[4] = createGdtDescriptor(0, 0xFFFFF, 0xF2, 0xC0); // User data segment

    GdtMetadata gdt_metadata;
    gdt_metadata.gdt_pointer = (uintptr_t)gdt_entries;
    gdt_metadata.gdt_size = (sizeof(GdtDescriptor) * 5 - 1);

    loadGdtr(&gdt_metadata);

    return;
}

void GdtError() {
    while (1) __asm__("hlt");
}

GdtDescriptor createGdtDescriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    /*
    limit = 20 bits; base = 32 bits; access = 8 btis; flags = 4 bits
    */
    if(limit > 0xFFFFF)
        GdtError();
    GdtDescriptor d = {0};

    d.low_limit = (uint16_t)(limit & 0xFFFFu);
    d.low_base  = (uint16_t)(base & 0xFFFFu);
    d.mid_base  = (uint8_t)((base >> 16) & 0xFFu);
    d.access_bytes = access;
    d.flags_and_high_limit =
        (uint8_t)((flags & 0xF0u) | ((limit >> 16) & 0x0Fu));
    d.high_base = (uint8_t)((base >> 24) & 0xFFu);

    return d;
}
