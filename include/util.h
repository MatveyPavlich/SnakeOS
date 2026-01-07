#include "stddef.h"
#include "stdint.h"

void *memcopy(void *dst, const void *src, size_t n);
void *memset(void *dst, int value, size_t n);

/* ATT syntax for writing 1 byte to a port (IO bus) */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

/* ATT syntax for reading 1 byte from a port (IO bus) */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/* Wait 1-4 ms. Useful for PIC remapping on old hardware. Linux uses port 0x80,
 * so am I.
 * */
static inline void io_wait(void)
{
    outb(0x80, 0);
}

/* String stuff */
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);

/* irq_save - helper to save interrupt context? */
static inline unsigned long irq_save(void)
{
        unsigned long flags;
        __asm__ volatile (
                "pushfq\n\t"
                "pop %0\n\t"
                "cli"
                : "=r"(flags)
                :
                : "memory"
        );
        return flags;
}

/* irq_restore - helper to restore interrupt context? */
static inline void irq_restore(unsigned long flags)
{
        __asm__ volatile (
                "push %0\n\t"
                "popfq"
                :
                : "r"(flags)
                : "memory"
        );
}
