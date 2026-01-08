/* Implementation of the console API */

#include "console.h"
#include "vga.h"
#include "stddef.h"

#define VGA_NUM_COLS 80
#define VGA_NUM_ROWS 25

/* struct console - Structure to store console's state.
 * @row:            Current cursor row.
 * @col:            Current cursor col.
 * @console_ops:    Console callbacks.
 */
struct console {
        size_t row;
        size_t col;
        const struct console_ops *ops;
};

static struct console kcon;

static void vga_console_putc(char c)
{
        if (c == '\n') {
                kcon.col = 0;
                kcon.row++;
                goto check_scroll;
        }

        vga_put_char(kcon.row, kcon.col, c);
        kcon.col++;

        if (kcon.col >= VGA_NUM_COLS) {
                kcon.col = 0;
                kcon.row++;
        }

        check_scroll:
        if (kcon.row >= VGA_NUM_ROWS) {
                vga_scroll_up();
                kcon.row = VGA_NUM_ROWS - 1;
        }
}

void console_putc(char c)
{
        kcon.ops->putc(c);
}

void console_write(const char *s)
{
        while (*s)
                console_putc(*s++);
}
static void vga_console_clear(void)
{
        vga_clear_screen();
        kcon.row = 0;
        kcon.col = 0;
}

static const struct console_ops vga_console_ops = {
        .putc  = vga_console_putc,
        .clear = vga_console_clear,
};

void console_init(void)
{
        kcon.row = 0;
        kcon.col = 0;
        kcon.ops = &vga_console_ops;

        kcon.ops->clear();
}
