/* API for IRQ controller drivers to register with the IRQ core subsystem.
 * For other drivers and core users (e.g. schedueler) see "interrupt.h"
 */

/* irq_chip - structure to represent all properties of an IRQ chip. Each driver
 *            will own their respective struct.
 * @name:            Name of the chip
 * @irq_init:        Int the chip
 * @irq_shutdown:    Mask all interrupts to prevent them going into the buffer.
 *                   Do this before switching to a different irq_chip
 * @irq_mask:        Mask a specific irq.
 * @irq_unmask:      Unmask a specific irq.
 * @irq_eoi:         Send an End Of Interrupt (EOI) signal to irq_chip
 */
struct irq_chip {
        const char *name;
        void (*irq_init)(void);
        void (*irq_shutdown)(void);
        void (*irq_mask)(int irq);
        void (*irq_unmask)(int irq);
        void (*irq_eoi)(int irq);
};

/* irq_set_chip - Register an interrupt controller with the IRQ core.
 * @chip:         Pointer to a chip struct owned by a IRQ controller driver.
 */
int irq_set_chip(struct irq_chip *chip);
