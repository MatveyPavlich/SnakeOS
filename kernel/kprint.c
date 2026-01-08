// kprint.c
#include "console.h"
#include "stdint.h"
#include "stddef.h"
#include <stdarg.h>

/* Examples: 
 * kprint("Value: %d\n", 1234);
 * kprint("Hex: %x\n", 0xBEEF);
 * kprint("Char: %c\n", 'A');
 * kprint("String: %s\n", "OSDev!");
 */

/* Convert integer to string (decimal) */
static void itoa_dec(int value, char* buffer)
{
        char temp[64];
        int i = 0;
        int neg = 0;

        if (value == 0) {
                buffer[0] = '0';
                buffer[1] = '\0';
                return;
        }

        if (value < 0) {
                neg = 1;
                value = -value;
        }

        while (value > 0) {
                temp[i++] = (value % 10) + '0';
                value /= 10;
        }

        if (neg) temp[i++] = '-';

        // reverse
        int j = 0;
        while (i > 0) {
                buffer[j++] = temp[--i];
        }
        buffer[j] = '\0';
}

/* Convert integer to string (hex) */
static void itoa_hex(unsigned int value, char* buffer)
{
        const char* digits = "0123456789ABCDEF";
        char temp[32];
        int i = 0;

        if (value == 0) {
                buffer[0] = '0';
                buffer[1] = '\0';
                return;
        }

        while (value > 0) {
                temp[i++] = digits[value % 16];
                value /= 16;
        }

        buffer[0] = '0';
        buffer[1] = 'x';

        int j = 2;
        while (i > 0) {
                buffer[j++] = temp[--i];
        }
        buffer[j] = '\0';
}

void kprint(const char* fmt, ...)
{
        va_list args;
        va_start(args, fmt);

        for (size_t i = 0; fmt[i] != '\0'; i++) {
                if (fmt[i] == '%') {
                        i++;
                        if (fmt[i] == 'd') {
                                uint64_t val = va_arg(args, int);
                                char buf[64];
                                itoa_dec(val, buf);
                                console_write(buf);
                        }
                        else if (fmt[i] == 'x') {
                                uint64_t val = va_arg(args, unsigned int);
                                char buf[64];
                                itoa_hex(val, buf);
                                console_write(buf);
                        }
                        else if (fmt[i] == 's') {
                                char* s = va_arg(args, char*);
                                console_write(s);
                        }
                        else if (fmt[i] == 'c') {
                                char c = (char) va_arg(args, int);
                                console_putc(c);
                        }
                        else {
                                console_putc('%');
                                console_putc(fmt[i]);
                        }
                } else {
                        console_putc(fmt[i]);
                }
        }

        va_end(args);
}
