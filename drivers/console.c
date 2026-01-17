/* Implementation of the console API */

#include "console.h"
#include "timer.h"
#include "stddef.h"
#include "vga.h"

#define NR_CONS      10
#define VGA_NUM_COLS 80
#define VGA_NUM_ROWS 25

struct console_ops;

/* struct console - Structure to store console's state for a specific console
 *                  type (e.g., VGA, framebuffer, etc.) or a specific console
 *                  instance of the same type (e.g., con_vga_1, con_vga_2, etc).
 * @name:           Identifier for a console.
 * @row:            Current cursor row.
 * @col:            Current cursor col.
 * @console_ops:    Console callbacks.
 */
struct console {
        const char *name;
        size_t row;
        size_t col;
        const struct console_ops *ops;
};

/* console_ops - Console backend operations owned by a specific console struct.
 * @putc:        Callback to write a character to a console.
 * @clear:       Callback to clear the screen.
 */
struct console_ops {
        void (*putc)(struct console *con, char c);
        void (*clear)(struct console *con);
};

static struct console consoles[NR_CONS];
static struct console *active_console;

/* console_vga_putc - Method for drawing a character in a VGA console.
 * @con:              Pointer to a specific vga console instance for which to
 *                    print the character (it will be used to fetch the cursor
 *                    state.
 * @c:                Character to be printed in the console.
 */
static void console_vga_putc(struct console *con, char c)
{
        if (c == '\n') {
                con->col = 0;
                con->row++;
                goto check_scroll;
        }

        vga_put_char(con->row, con->col, c);
        con->col++;

        if (con->col >= VGA_NUM_COLS) {
                con->col = 0;
                con->row++;
        }

        check_scroll:
        if (con->row >= VGA_NUM_ROWS) {
                vga_scroll_up();
                con->row = VGA_NUM_ROWS - 1;
        }
}

/* console_vga_clear_screen - Method for clearing a console screen.
 * @con:                      Pointer to a specific vga console instance for
 *                            which to clearn the screen and zero the cursor.
 */
static void console_vga_clear_screen(struct console *con)
{
        if (con == active_console)
                vga_clear_screen();
        else {
                /* TODO: Impelement a console_screen_dirty_flag() to indicate
                 * that a console should be cleaned
                 */
        }
        con->row = 0;
        con->col = 0;
}

void console_putc(char c)
{
        if (!active_console || !active_console->ops)
                return;

        active_console->ops->putc(active_console, c);
}

void console_write(const char *s)
{
        if (!s) return;
        while (*s)
                console_putc(*s++);
}

void console_backspace(void)
{
        struct console *con = active_console;

        if (!con)
                return;

        if (con->col == 0) {
                if (con->row == 0)
                        return;

                con->row--;
                con->col = VGA_NUM_COLS - 1;
        } else {
                con->col--;
        }

        /* overwrite character at new cursor position */
        vga_put_char(con->row, con->col, ' ');
}

void console_move_cursor(int dx, int dy)
{
        struct console *con = active_console;

        if (!con)
                return;

        int new_col = (int)con->col + dx;
        int new_row = (int)con->row + dy;

        if (new_col < 0)
                new_col = 0;
        if (new_col >= VGA_NUM_COLS)
                new_col = VGA_NUM_COLS - 1;

        if (new_row < 0)
                new_row = 0;
        if (new_row >= VGA_NUM_ROWS)
                new_row = VGA_NUM_ROWS - 1;

        con->col = (size_t)new_col;
        con->row = (size_t)new_row;
}

void console_clear(void)
{
        if (!active_console || !active_console->ops)
                return;

        active_console->ops->clear(active_console);
}

/* VGA-only overlay, bypasses console abstraction intentionally.
 * TODO: create a console_draw_clock() to manage timer drawing.
 * TODO: Solve races (this will be executed from the timer context and can race
 *       against the console imput.
 */
static void vga_draw_clock(void)
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
        buf[7] = '0' + (s % 10);
        buf[8] = '\0';

        int row = VGA_NUM_ROWS - 1;
        int col = VGA_NUM_COLS - 8;
        for (int i = 0; i < 8; i++)
                vga_put_char(row, col + i, buf[i]);
}

static const struct console_ops console_vga_ops = {
        .putc  = console_vga_putc,
        .clear = console_vga_clear_screen,
};

void console_init(void)
{
        consoles[0] = (struct console) {
                .name = "kcon_vga_1",
                .row = 0,
                .col = 0,
                .ops = &console_vga_ops,
        };

        active_console = &consoles[0];
        active_console->ops->clear(active_console);

        timer_register_secs_hook(vga_draw_clock);
}
