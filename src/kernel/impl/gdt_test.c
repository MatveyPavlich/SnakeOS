#include "stddef.h"
#include "stdint.h"
#include "gdt.h"
#include "util.h"   // for memset
#include "kprint.h"

#define GDT_SEGMENT_COUNT 5   // null + kernel code/data + user code/data
#define GDT_TSS_COUNT     2   // TSS takes 16 bytes = 2 entries
#define GDT_ENTRY_COUNT   (GDT_SEGMENT_COUNT + GDT_TSS_COUNT)

extern void loadGdtr(GdtMetadata *m);
extern void loadLtr(uint16_t selector); // you’ll need a small asm stub for this

// ---- Actual TSS ----
Tss64Entry tss __attribute__((aligned(16))); // 1 TSS for single core

// ---- GDT ----
static uint64_t gdt[GDT_ENTRY_COUNT] __attribute__((aligned(16)));

// ---- Helpers ----
static GdtSegmentDescriptor createGdtSegmentDescriptor(uint32_t base, uint32_t limit,
                                                       uint8_t access, uint8_t flags) {
    if (limit > 0xFFFFF) {
        kprint("Invalid gdt entry \n");
        while (1) __asm__("hlt");
    }

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

static GdtSystemDescriptor createGdtSystemDescriptor(uint64_t base, uint32_t limit) {
    GdtSystemDescriptor d;
    d.limit_low  = limit & 0xFFFF;
    d.base_low   = base & 0xFFFF;
    d.base_mid   = (base >> 16) & 0xFF;
    d.access     = 0x89;   // present, type=available 64-bit TSS
    d.gran       = (limit >> 16) & 0x0F;
    d.base_high  = (base >> 24) & 0xFF;
    d.base_upper = (base >> 32) & 0xFFFFFFFF;
    d.reserved   = 0;
    return d;
}

// ---- Init ----
void gdtInit(uint64_t kernel_stack_top, uint64_t df_stack_top) {
    // 1. Init TSS
    memset(&tss, 0, sizeof(Tss64Entry));
    tss.rsp0 = kernel_stack_top;
    tss.ist1 = df_stack_top; 
    tss.iomap = sizeof(Tss64Entry); // no I/O bitmap

    // 2. Build GDT
    GdtSegmentDescriptor seg;

    // Null
    seg = createGdtSegmentDescriptor(0, 0, 0, 0);
    memcpy(&gdt[0], &seg, sizeof(seg));

    // Kernel code
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0x9A, 0x20);
    memcpy(&gdt[1], &seg, sizeof(seg));

    // Kernel data
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0x92, 0xC0);
    memcpy(&gdt[2], &seg, sizeof(seg));

    // User code
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0xFA, 0x20);
    memcpy(&gdt[3], &seg, sizeof(seg));

    // User data
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0xF2, 0xC0);
    memcpy(&gdt[4], &seg, sizeof(seg));

    // TSS (16 bytes → 2 entries)
    GdtSystemDescriptor sys = createGdtSystemDescriptor((uint64_t)&tss, sizeof(Tss64Entry) - 1);
    memcpy(&gdt[5], &sys, sizeof(sys));

    // 3. Load GDT
    GdtMetadata gdt_metadata;
    gdt_metadata.gdt_pointer = (uintptr_t)gdt;
    gdt_metadata.gdt_size = sizeof(gdt) - 1;
    loadGdtr(&gdt_metadata);

    // 4. Load TR (TSS selector at offset 0x28, i.e., index 5*8)
    loadLtr(5 << 3);
}
