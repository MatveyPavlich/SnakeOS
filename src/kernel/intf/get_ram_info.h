#include "stddef.h"
#include "stdint.h"


struct ram_entry {
        uint64_t base;
        uint64_t length;
        uint32_t type;
} __attribute__((packed));

struct ram_entry *ram_entries_by_bios = (struct ram_entry *)0x86000;


