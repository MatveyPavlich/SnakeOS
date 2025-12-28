/* API interface for the i8259A controller (PIC) */
#include "irq.h"

/* Expose the init hook to perform registration */
void irq_arch_init(void);
