/* tty subsystem implementation */

#include "console.h"
#include "input.h"
#include "stddef.h"
#include <stdbool.h>
#include "tty.h"

#define TTY_BUF_SIZE 256

/* struct tty - structure to represent a terminal line.
 * buf:         Characters that the line contains.
 * len:         Number characters on the line.
 * cursor:      Cursor position on the line.
 * echo:        True when text is displayed on the screen, false for when its
 *              hidden (e.g., password line).
 */
struct tty {
        char   buf[TTY_BUF_SIZE];
        size_t len;
        size_t cursor;
        bool   echo;
};

/* Active editable line in the terminal */
struct tty tty_active = {
        .len = 0,
        .cursor = 0,
        .echo = true,
};

/* tty_backspace - text deletion function. */
static void tty_backspace(void)
{
        if (tty_active.cursor == 0)
                return;

        /* Updated active tty line state */
        for (size_t i = tty_active.cursor - 1; i < tty_active.len - 1; i++)
                tty_active.buf[i] = tty_active.buf[i + 1];
        tty_active.cursor--;
        tty_active.len--;

        /* Redraw the new tty state in the console */
        if (!tty_active.echo)
                return;
        else {
                console_move_cursor(-1, 0);

                /* Redraw tail and clear the last character */
                for (size_t i = tty_active.cursor; i < tty_active.len; i++)
                        console_putc(tty_active.buf[i]);
                console_putc(' ');

                /* Align console cursor with tty's cursor by shift latter to the
                 * left */
                size_t chars_printed = tty_active.len - tty_active.cursor + 1;
                console_move_cursor(chars_printed * (-1), 0);
        }
}

static void tty_cursor_left(void)
{
        if (tty_active.cursor > 0) {
                tty_active.cursor--;
                if (tty_active.echo)
                        console_move_cursor(-1, 0);
        }
}

static void tty_cursor_right(void)
{
        if (tty_active.cursor < tty_active.len) {
                tty_active.cursor++;
                if (tty_active.echo)
                        console_move_cursor(1, 0);
        }
}

static void tty_insert_char(char c)
{
        if (tty_active.len >= TTY_BUF_SIZE)
                return;

        for (size_t i = tty_active.len; i > tty_active.cursor; i--)
                tty_active.buf[i] = tty_active.buf[i - 1];

        tty_active.buf[tty_active.cursor] = c;
        tty_active.cursor++;
        tty_active.len++;

        if (tty_active.echo) {
                console_putc(c);

                /* redraw tail if inserting in middle */
                for (size_t i = tty_active.cursor; i < tty_active.len; i++)
                        console_putc(tty_active.buf[i]);

                /* move cursor back */
                for (size_t i = tty_active.cursor; i < tty_active.len; i++)
                        console_move_cursor(-1, 0);
        }
}

static void tty_newline(void)
{
        if (tty_active.echo)
                console_putc('\n');

        /* In a real OS, you'd wake readers here */

        tty_active.len = 0;
        tty_active.cursor = 0;
}

void tty_handle_key(const struct key_event *ev)
{
        if (ev->action != KEY_PRESS)
                return;

        /* Printable character */
        if (ev->ascii) {
                tty_insert_char(ev->ascii);
                return;
        }

        switch (ev->key) {
                case KEY_BACKSPACE:
                        tty_backspace();
                        break;

                case KEY_LEFT:
                        tty_cursor_left();
                        break;

                case KEY_RIGHT:
                        tty_cursor_right();
                        break;

                case KEY_ENTER:
                        tty_newline();
                        break;

                default:
                        break;
        }
}
