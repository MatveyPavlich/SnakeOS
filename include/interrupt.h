#include "stdint.h"

/* interrupt_frame - Structure to capture the CPU state before the interrupt
 * entry
 */
struct interrupt_frame {
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t error_code;
};
