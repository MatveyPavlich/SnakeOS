#include "stddef.h"
#include "stdint.h"

// Copy n bytes from src → dst
void *memcopy(void *dst, const void *src, size_t n)
{
        uint8_t *d = (uint8_t *)dst;
        const uint8_t *s = (const uint8_t *)src;
        while (n--) *d++ = *s++;
        return dst;
}

// Set n bytes at dst to value (unsigned char)
void *memset(void *dst, int value, size_t n)
{
        uint8_t *d = (uint8_t *)dst;
        uint8_t val = (uint8_t)value;
        while (n--) *d++ = val;
        return dst;
}
