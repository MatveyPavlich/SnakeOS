/* Console subsystem interface. Code inspired and adopted from the linux kernel.
 * See include/linux/console.h
 * Console subsustem is responsible for calling the VGA module. It is a
 * penultimate step before writing characters to the memory:
 *       kprint("Yo\n"); -> console_putc('H') -> vga_putc(0,0,'H') -> 0xB8000
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>

/*
 * console_ops - low-level console backend operations
 *
 * This describes *how* characters are rendered.
 * Policy (cursor, wrapping, scrolling) lives in console.c.
 */
struct console_ops {
        void (*putc)(char c);
        void (*clear)(void);
};

/* Initialise the kernel console */
void console_init(void);

/* Output a single character */
void console_putc(char c);

/* Output a NUL-terminated string */
void console_write(const char *s);

#endif /* CONSOLE_H */
