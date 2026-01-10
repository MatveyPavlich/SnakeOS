/* tty subsytem implementation */

#include "tty.h"

/* Forward declarations */
static void tty_insert_char(struct key_event);
static void tty_backspace(void);
static void tty_cursor_left();
static void tty_cursor_right();
static void tty_cursor_up();
static void tty_cursor_down();
static void tty_newline();

void tty_input(struct key_event ev)
{
        switch (ev.code) {
                case KEY_CHAR:
                        tty_insert_char(ev.ch);
                        break;

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
