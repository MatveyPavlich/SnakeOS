/* TODO: cleanup according to the linux coding style (which I now use) */

#include "kprint.h"
#include "stddef.h"
#include "stdint.h"
#include "util.h"

#define GDT_SEGMENT_DESCRIPTOR_NUM 5 /* null, kernel code/data, user code/data */
#define GDT_SYSTEM_DESCRIPTOR_COUNT  1  
#define GDT_ENTRY_COUNT              (GDT_SEGMENT_DESCRIPTOR_NUM + GDT_SYSTEM_DESCRIPTOR_COUNT * 2)

/* struct gdt_seg_desc  -  Structure to represent a gdt segment descriptor.
 * @low_limit:             Bits [15:0] of segment's limit.
 * @low_base:              Bits [15:0] of segment's base.
 * @mid_base:              Bits [23:16] of segment's base.
 * @access_bytes:          P|DPL|S|Type (see intel's manual).
 * @flags_and_high_limit:  Flags(high nibble) | limit[19:16](low nibble).
 * high_base:              Bits [31:24] of segment's base.
 */
struct gdt_seg_desc {
        uint16_t low_limit;
        uint16_t low_base;
        uint8_t  mid_base;
        uint8_t  access_bytes;
        uint8_t  flags_and_high_limit;
        uint8_t  high_base;
} __attribute__((packed));

/* struct gdt_sys_desc  -  Structure to represent a gdt system descriptor. In
 *                         64-bits they are extended to x2 segment descriptors
 *                         (16 bytes)
 * @low_limit:             Bits [15:0] of segment's limit.
 * @low_base:              Bits [15:0] of segment's base.
 * @mid_base:              Bits [23:16] of segment's base.
 * @access_bytes:          P|DPL|S|Type (see intel's manual).
 * @flags_and_high_limit:  Flags(high nibble) | limit[19:16](low nibble).
 * high_base:              Bits [31:24] of segment's base.
 */
struct gdt_sys_desc {
        uint16_t limit_low;            // Limit 0:15
        uint16_t base_low;             // Base 0:15
        uint8_t  base_mid;             // Base 16:23
        uint8_t  access;               // Type(0x9 / 0xB) | P=1 | DPL | S=0
        uint8_t  gran;                 // Limit 16:19 | flags
        uint8_t  base_high;            // Base 24:31

        // extra 8 bytes for 64-bit TSS
        uint32_t base_upper;           // Base 32:63
        uint32_t reserved;             // Must be 0
} __attribute__((packed));

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

extern void loadGdtr(GdtMetadata *m);
extern void loadLtr(uint16_t selector);

static uint64_t gdt[GDT_ENTRY_COUNT] __attribute__((aligned(16))); // GDT table
static Tss64Entry tss __attribute__((aligned(16))); // TSS table (u should have 1 TSS per core => for now make the OS single core)
static uint8_t df_stack[4096] __attribute__((aligned(16))); // 4 KiB stack for TSS entry to be used in double faults 
static uint8_t kernel_stack[16384] __attribute__((aligned(16))); // 16 KiB kernel stack for TSS entry to be used for kernel

static struct gdt_sys_desc gdt_generate_sys_desc(uint64_t base, uint32_t limit)
{
        struct gdt_sys_desc d = {
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
        }

        return d;
}

static struct gdt_seg_desc_t gdt_generate_seg_desc(uint32_t base,
                uint32_t limit,
                uint8_t access,
                uint8_t flags)
{
        /* limit = 20 bits; base = 32 bits; access = 8 btis; flags = 4 bits */

        if (limit > 0xFFFFF) {
                kprint("Invalid gdt entry \n");
                while (1) __asm__("hlt");
        }

        struct gdt_seg_desc_t d = {
                d.low_limit = (uint16_t)(limit & 0xFFFFu);
                d.low_base  = (uint16_t)(base & 0xFFFFu);
                d.mid_base  = (uint8_t)((base >> 16) & 0xFFu);
                d.access_bytes = access;
                d.flags_and_high_limit =
                        (uint8_t)((flags & 0xF0u) | ((limit >> 16) & 0x0Fu));
                d.high_base = (uint8_t)((base >> 24) & 0xFFu);
        };

        return d;
}

void gdt_init() {

        /* 1. Init TSS */
        memset(&tss, 0, sizeof(Tss64Entry));
        tss.rsp0 = ((uint64_t)(kernel_stack + sizeof(kernel_stack)));
        tss.ist1 = ((uint64_t)(df_stack + sizeof(df_stack))); 
        tss.iomap = sizeof(Tss64Entry); // no I/O bitmap

        // 2. Build GDT
        struct gdt_seg_desc_t seg;
        seg = gdt_generate_seg_desc(0, 0, 0, 0);             // Null
        memcopy(&gdt[0], &seg, sizeof(seg));
        seg = gdt_generate_seg_desc(0, 0xFFFFF, 0x9A, 0x20); // Kernel code
        memcopy(&gdt[1], &seg, sizeof(seg));
        seg = gdt_generate_seg_desc(0, 0xFFFFF, 0x92, 0xC0); // Kernel data
        memcopy(&gdt[2], &seg, sizeof(seg));
        seg = gdt_generate_seg_desc(0, 0xFFFFF, 0xFA, 0x20); // User code
        memcopy(&gdt[3], &seg, sizeof(seg));
        seg = gdt_generate_seg_desc(0, 0xFFFFF, 0xF2, 0xC0); // User data
        memcopy(&gdt[4], &seg, sizeof(seg));

        struct gdt_sys_desc_t sys = createGdtSystemDescriptor((uint64_t)&tss, sizeof(Tss64Entry) - 1); // TSS (16 bytes → 2 entries)
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

