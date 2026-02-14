#include "stddef.h"
#include "stdint.h"

struct ram_entry {
        uint64_t base;
        uint64_t length;
        uint32_t type;
} __attribute__((packed));

void print_ram_map(void);
