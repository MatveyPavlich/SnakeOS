#pragma once
#include "stdint.h"

typedef struct {
    uint16_t offset_low;   // bits 0..15 of handler function address
    uint16_t selector;     // code segment selector in GDT
    uint8_t  ist;          // bits 0..2 = IST index, rest zero
    uint8_t  type_attr;    // gate type, DPL, P
    uint16_t offset_mid;   // bits 16..31 of handler address
    uint32_t offset_high;  // bits 32..63 of handler address
    uint32_t zero;         // reserved
} __attribute__((packed)) IdtEntry;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) IdtDescriptor;

#define IDT_SIZE 256   // 256 possible vectors

void idtInit(void);
