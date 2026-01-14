/* TODO: cleanup according to the linux coding style (which I now use) */

#include "kprint.h"
#include "stddef.h"
#include "stdint.h"
#include "util.h"

/* Access byte bits */
#define DESC_P          0x80    /* Present */
#define DESC_DPL0       0x00
#define DESC_DPL3       0x60
#define DESC_S          0x10    /* Descriptor type (0 = system, 1 = code/data) */

/* Code/Data types */
#define DESC_CODE       0x08
#define DESC_DATA       0x00
#define DESC_EXEC       0x08
#define DESC_RW         0x02
#define DESC_ACCESSED   0x01

/* Typical segments */
#define GDT_KERNEL_CODE (DESC_P | DESC_S | DESC_EXEC | DESC_RW)
#define GDT_KERNEL_DATA (DESC_P | DESC_S | DESC_RW)
#define GDT_USER_CODE   (DESC_P | DESC_S | DESC_EXEC | DESC_RW | DESC_DPL3)
#define GDT_USER_DATA   (DESC_P | DESC_S | DESC_RW | DESC_DPL3)

/* TSS flags */
#define TSS_AVAIL_64   0x09
#define TSS_BUSY_64    0x0B
#define TSS_DESC_ACCESS (DESC_P | TSS_AVAIL_64)

#define GDT_SEG_DESC_NUM 5 /* null, kernel code/data, user code/data */
#define GDT_SYS_DESC_NUM 1  
#define GDT_ENTRY_COUNT  (GDT_SEG_DESC_NUM + GDT_SYS_DESC_NUM * 2)

/* struct gdt_seg_desc  -  Structure to represent a gdt segment descriptor.
 * @limit_low:             Bits [15:0] of segment's limit.
 * @base_low:              Bits [15:0] of segment's base.
 * @base_mid:              Bits [23:16] of segment's base.
 * @access_bytes:          P|DPL|S|Type (see intel's manual).
 * @flags_and_limit_high:  Flags(high nibble) | limit[19:16](low nibble).
 * @base_high:             Bits [31:24] of segment's base.
 */
struct gdt_seg_desc {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t  base_mid;
        uint8_t  access_bytes;
        uint8_t  flags_and_limit_high;
        uint8_t  base_high;
} __attribute__((packed));

/* struct gdt_sys_desc  -  Structure to represent a gdt system descriptor. In
 *                         64-bits they are extended to x2 segment descriptors
 *                         (16 bytes).
 * @limit_low:             Bits [15:0] of segment's limit.
 * @base_low:              Bits [15:0] of segment's base.
 * @base_mid:              Bits [23:16] of segment's base.
 * @access:                Type(0x9 / 0xB) | P=1 | DPL | S=0
 * @gran:                  Limit 16:19 | flags
 * @base_high:             Bits [31:24] of segment's base.
 * -- Extra 8 bytes for 64-bit TSS --
 * @base_upper:            Bits [63:32] of segment's base.
 * @reserved:              Reserved memo that should be 0.
 */
struct gdt_sys_desc {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t  base_mid;
        uint8_t  access; 
        uint8_t  gran; 
        uint8_t  base_high;
        uint32_t base_upper;
        uint32_t reserved;
} __attribute__((packed));

/* struct tss_64 - Structure of the 64-bit TSS to be stored in memory and
 *                 referenced by a CPU through the GDT (you'll need to create a
 *                 dedicated GDT descriptor for TSS). Note that you should have
 *                 1 tss entry per core. So, while the OS is single core just
 *                 have 1 of them.
 * @reserved0-3:   Zero out.
 * @rsp0:          Ring 0 stack ptr (i.e., rsp reg).
 * @rsp1:          Ring 1 stack ptr (i.e., rsp reg). Zero out since not used.
 * @rsp2:          Ring 2 stack ptr (i.e., rsp reg). Zero out since not used.
 * @ist1-7:        Interrupt stack table.
 * @iomap:         I/O permissions bitmap.
 */
struct tss_64 {
        uint32_t reserved0;
        uint64_t rsp0;
        uint64_t rsp1;
        uint64_t rsp2;
        uint64_t reserved1;
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

} __attribute__((packed));

/* struct gdt_metadata - structure to be passed into the CPU and load the GDT.
 *                       Note that the CPU copies this data and stores itself,
 *                       so we don't need to keep this struct.
 * @gdt_size:            Size of the GDT table in bytes.
 * @gdt_table_pointer:   Pointer to the GDT table start.
 */
struct gdt_metadata {
        uint16_t  gdt_size;
        uintptr_t gdt_table_pointer;

} __attribute__((packed));

extern void gdt_load(struct gdt_metadata *m);
extern void gdt_load_tss(uint16_t gdt_tss_offset);

static uint64_t gdt_table[GDT_ENTRY_COUNT] __attribute__((aligned(16)));
static struct tss_64 tss __attribute__((aligned(16))); 

/* 4KiB emergency stack for double faults */
static uint8_t emergency_stack[4096] __attribute__((aligned(16)));
/* 16KiB kernel stack */
static uint8_t kernel_stack[16384] __attribute__((aligned(16)));

static struct gdt_sys_desc gdt_generate_sys_desc(uint64_t base, uint32_t limit)
{
        struct gdt_sys_desc d = {
                d.limit_low  = limit & 0xFFFF;
                d.base_low   = base & 0xFFFF;
                d.base_mid   = (base >> 16) & 0xFF;
                d.access     = TSS_DESC_ACCESS;
                d.gran       = ((limit >> 16) & 0x0F);
                d.gran      |= 0x00; // G=0, AVL=0, L=0, DB=0 (must be zero for TSS)
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
                d.limit_low = (uint16_t)(limit & 0xFFFFu);
                d.base_low  = (uint16_t)(base & 0xFFFFu);
                d.base_mid  = (uint8_t)((base >> 16) & 0xFFu);
                d.access_bytes = access;
                d.flags_and_limit_high = (uint8_t)((flags & 0xF0u)
                                         | ((limit >> 16) & 0x0Fu));
                d.base_high = (uint8_t)((base >> 24) & 0xFFu);
        };

        return d;
}

void gdt_init() {

        /* Init TSS */
        memset(&tss, 0, sizeof(tss));
        tss.rsp0 = ((uint64_t)(kernel_stack + sizeof(kernel_stack)));
        tss.ist1 = ((uint64_t)(emergency_stack + sizeof(df_stack))); 
        tss.iomap = sizeof(tss); /* no I/O bitmap */

        /* GDT entries */
        gdt_table[0] = gdt_generate_seg_desc(0, 0, 0, 0); /* Null entry */

        gdt[1] = gdt_generate_seg_desc(0,
                                       0xFFFFF,
                                       GDT_KERNEL_CODE,
                                       GDT_FLAGS_CODE64);

        gdt[2] = gdt_generate_seg_desc(0,
                                       0xFFFFF,
                                       GDT_KERNEL_DATA,
                                       GDT_FLAGS_DATA);

        gdt[3] = gdt_generate_seg_desc(0,
                                       0xFFFFF,
                                       GDT_USER_CODE,
                                       GDT_FLAGS_CODE64);

        gdt[4] = gdt_generate_seg_desc(0,
                                       0xFFFFF,
                                       GDT_USER_DATA,
                                       GDT_FLAGS_DATA);

        /* GDT descriptor for the TSS is two slots */
        struct gdt_sys_desc_t sys = gdt_generate_sys_desc((uint64_t)&tss,
                                                          sizeof(tss) - 1);
        memcopy(&gdt[5], &sys, sizeof(sys));

        /* Load GDT & TSS */
        struct gdt_metadata gdt_meta = {
                .gdt_table_pointer = (uintptr_t)gdt_table;
                .gdt_size = sizeof(gdt_table) - 1;
        }
        gdt_load(&gdt_meta);
        gdt_load_tss(5 << 3); /* TSS at index 5 => offset = 0x28 (5*8) */

        return;
}

