/* tty subsytem implementation */

#include "console.h"
#include "input.h"
#include "stddef.h"
#include <stdbool.h>
#include "tty.h"

#define TTY_BUF_SIZE 256

static struct {
        char   buf[TTY_BUF_SIZE];
        size_t len;       /* number of chars in buffer */
        size_t cursor;    /* 0 <= cursor <= len */
        bool   echo;
} tty = {
        .len = 0,
        .cursor = 0,
        .echo = true,
};


static void tty_backspace(void)
{
        if (tty.cursor == 0)
                return;

        for (size_t i = tty.cursor - 1; i < tty.len - 1; i++)
                tty.buf[i] = tty.buf[i + 1];

        tty.cursor--;
        tty.len--;

        if (tty.echo)
                console_backspace();
}


static void tty_cursor_left(void)
{
        if (tty.cursor > 0) {
                tty.cursor--;
                if (tty.echo)
                        console_move_cursor(-1, 0);
        }
}

static void tty_cursor_right(void)
{
        if (tty.cursor < tty.len) {
                tty.cursor++;
                if (tty.echo)
                        console_move_cursor(1, 0);
        }
}

static void tty_insert_char(char c)
{
        if (tty.len >= TTY_BUF_SIZE)
                return;

        for (size_t i = tty.len; i > tty.cursor; i--)
                tty.buf[i] = tty.buf[i - 1];

        tty.buf[tty.cursor] = c;
        tty.cursor++;
        tty.len++;

        if (tty.echo) {
                console_putc(c);

                /* redraw tail if inserting in middle */
                for (size_t i = tty.cursor; i < tty.len; i++)
                        console_putc(tty.buf[i]);

                /* move cursor back */
                for (size_t i = tty.cursor; i < tty.len; i++)
                        console_move_cursor(-1, 0);
        }
}


static void tty_newline(void)
{
        if (tty.echo)
                console_putc('\n');

        /* In a real OS, you'd wake readers here */

        tty.len = 0;
        tty.cursor = 0;
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
