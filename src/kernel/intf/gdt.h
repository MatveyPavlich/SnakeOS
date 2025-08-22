#include "stddef.h"
#include "stdint.h"

typedef struct {
    uint16_t low_limit;            // limit[15:0]
    uint16_t low_base;             // base[15:0]
    uint8_t  mid_base;             // base[23:16]
    uint8_t  access_bytes;         // P|DPL|S|Type
    uint8_t  flags_and_high_limit; // Flags(high nibble) | limit[19:16](low nibble)
    uint8_t  high_base;            // base[31:24]

} __attribute__((packed)) GdtDescriptor;

typedef struct {
    uint16_t  gdt_size;
    uintptr_t gdt_pointer;

} __attribute__((packed)) GdtMetadata;

void gdtInit();
GdtDescriptor createGdtDescriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);