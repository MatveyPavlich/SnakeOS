/* Console subsystem interface. Code inspired and adopted from the linux kernel.
 * See include/linux/console.h
 * Console subsustem is responsible for calling the VGA module. It is a
 * penultimate step before writing characters to the memory:
 *       kprint("Yo\n"); -> console_putc('H') -> vga_putc(0,0,'H') -> 0xB8000
 */

#include "stdint.h"

/* struct console_ops - Callback interface for a console
 * @con_putc:           Emit a single character to VGA.
 * @con_clear:          Clear a single character from VGA.
 * @con_scroll:         Scroll.
 */
struct console_ops {
        const char *owner;
        void (*con_putc)(struct vc_data *, int);
        void (*con_clear)(struct vc_data *, int, int, int, int);
        void (*con_scroll)(struct vc_data *, int, int, int, int);
};
