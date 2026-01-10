/* Console subsystem interface. Code inspired and adopted from the linux kernel.
 * See include/linux/console.h
 * Console subsustem is responsible for calling the VGA module. It is the
 * penultimate step before writing characters to the memory:
 *       kprint("Yo\n"); -> console_putc('Y') -> vga_putc(0,0,'Y') -> 0xB8000
 */

#ifndef CONSOLE_H
#define CONSOLE_H

/* Initialise the kernel console by clearing the screen and creating a cursor
 * position state
 */
void console_init(void);

/* Output a single character */
void console_putc(char c);

/* Output a NUL-terminated string */
void console_write(const char *s);

#endif /* CONSOLE_H */
