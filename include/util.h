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
