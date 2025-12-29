/* Public API for core IRQ users and drivers */

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

typedef void (*irq_handler_t)(int irq, struct interrupt_frame *, void *dev_id);

/* irq_request - Add a handler to an interrupt line.
 * @irq:         The interrupt line to allocate.
 * @handler:     Function to call when IRQ occurs.
 * @dev:	 A cookie passed to the handler function.
 * Caller must check if the allocation was successful.
 */
int irq_request(int irq, irq_handler_t handler, void *dev);
