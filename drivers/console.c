/* Implementation of the console API */

#include "console.h"
#include "vga.h"
#include "stddef.h"
#include "timer.h"

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

        timer_register_secs_hook(console_draw_clock);
}

void console_draw_clock(void)
{
        uint64_t seconds = timer_get_seconds();

        uint64_t h = seconds / 3600;
        uint64_t m = (seconds / 60) % 60;
        uint64_t s = seconds % 60;

        // Format hh:mm:ss
        char buf[9];
        buf[0] = '0' + (h / 10) % 10;
        buf[1] = '0' + (h % 10);
        buf[2] = ':';
        buf[3] = '0' + (m / 10);
        buf[4] = '0' + (m % 10);
        buf[5] = ':';
        buf[6] = '0' + (s / 10);
        buf[7] = '0' + (seconds % 10);
        buf[8] = '\0';

        console_draw_at(VGA_ROWS - 1, VGA_COLS - 8, buf);
}

