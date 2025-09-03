#include "stddef.h"
#include "stdint.h"
#include "gdt.h"
#include "util.h"
#include "kprint.h"



#define GDT_SEGMENT_DESCRIPTOR_COUNT 5 // null + kernel code/data + user code/data
#define GDT_SYSTEM_DESCRIPTOR_COUNT  1  
#define GDT_ENTRY_COUNT              (GDT_SEGMENT_DESCRIPTOR_COUNT + GDT_SYSTEM_DESCRIPTOR_COUNT * 2)



extern void loadGdtr(GdtMetadata *m);
extern void loadLtr(uint16_t selector);



static uint64_t     gdt[GDT_ENTRY_COUNT]   __attribute__((aligned(16))); // GDT table
static Tss64Entry   tss                    __attribute__((aligned(16))); // TSS table (u should have 1 TSS per core => for now make the OS single core)
static uint8_t      df_stack[4096]         __attribute__((aligned(16))); // 4 KiB stack for TSS entry to be used in double faults 
static uint8_t      kernel_stack[16384]    __attribute__((aligned(16))); // 16 KiB kernel stack for TSS entry to be used for kernel



void gdtInit() {
   
    // 1. Init TSS
    memset(&tss, 0, sizeof(Tss64Entry));
    tss.rsp0 = ((uint64_t)(kernel_stack + sizeof(kernel_stack)));
    tss.ist1 = ((uint64_t)(df_stack + sizeof(df_stack))); 
    tss.iomap = sizeof(Tss64Entry); // no I/O bitmap

    // 2. Build GDT
    GdtSegmentDescriptor seg;
    seg = createGdtSegmentDescriptor(0, 0, 0, 0);             // Null
    memcopy(&gdt[0], &seg, sizeof(seg));
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0x9A, 0x20); // Kernel code
    memcopy(&gdt[1], &seg, sizeof(seg));
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0x92, 0xC0); // Kernel data
    memcopy(&gdt[2], &seg, sizeof(seg));
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0xFA, 0x20); // User code
    memcopy(&gdt[3], &seg, sizeof(seg));
    seg = createGdtSegmentDescriptor(0, 0xFFFFF, 0xF2, 0xC0); // User data
    memcopy(&gdt[4], &seg, sizeof(seg));

    GdtSystemDescriptor sys = createGdtSystemDescriptor((uint64_t)&tss, sizeof(Tss64Entry) - 1); // TSS (16 bytes → 2 entries)
    memcopy(&gdt[5], &sys, sizeof(sys));

    // 3. Load GDT
    GdtMetadata gdt_metadata;
    gdt_metadata.gdt_pointer = (uintptr_t)gdt;
    gdt_metadata.gdt_size = sizeof(gdt) - 1;
    loadGdtr(&gdt_metadata);

    // 4. Load TR 
    loadLtr(5 << 3);                                           // TSS descriptor at index 5 => offset = 0x28 (5*8)
    
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

GdtSegmentDescriptor createGdtSegmentDescriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    /*
    limit = 20 bits; base = 32 bits; access = 8 btis; flags = 4 bits
    */
    if (limit > 0xFFFFF) {
        kprintf("Invalid gdt entry \n");
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
