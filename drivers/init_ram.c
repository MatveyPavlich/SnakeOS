#include "kprint.h"
#include "stdint.h"
#include "stddef.h"

struct ram_entry {
        uint64_t base;
        uint64_t length;
        uint32_t type;
} __attribute__((packed));

struct ram_entry *ram_entries_by_bios = (struct ram_entry *)0x86000;
uint16_t *ram_entries = (uint16_t *)0x85FFD;

void print_ram_map(void)
{
        kprint("Entries count: %d\n", *ram_entries);
        for (int i = 0; i < *ram_entries; i++) {
                struct ram_entry e = ram_entries_by_bios[i];
                kprint("Entry %d:", i);
                kprint(" Base = %x", (unsigned int)(e.base & 0xFFFFFFFF));
                kprint(" Length = %x", (unsigned int)(e.length & 0xFFFFFFFF));
                kprint(" Type = %d\n", (int)e.type);
        }
}
