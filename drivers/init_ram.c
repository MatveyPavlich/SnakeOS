#include "init_ram.h"
#include "kprint.h"

struct ram_entry *ram_entries_by_bios = (struct ram_entry *)0x86000;

void print_ram_map(void) {
    for (int i = 0; i < 10; i++) {
        struct ram_entry e = ram_entries_by_bios[i];
        kprintf("Entry %d:", i);
        kprintf(" Base = %x", (unsigned int)(e.base & 0xFFFFFFFF));
        kprintf(" Length = %x", (unsigned int)(e.length & 0xFFFFFFFF));
        kprintf(" Type = %d\n", (int)e.type);
    }
}
