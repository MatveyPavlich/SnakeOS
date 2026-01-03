#include "cdev.h"
#include "kprint.h"
#include <stdbool.h>
#include "stddef.h"
#include "tty.h"

#define TTY_BUF_SIZE 128

/* Keyboard device */
static struct cdev *kbd;

/* Line buffer */
static char tty_buf[TTY_BUF_SIZE];
static size_t tty_len = 0;

/* Whether a full line is available */
static bool line_ready = false;

void tty_init(void)
{
        kbd = cdev_get("keyboard");
        if (!kbd || !kbd->ops || !kbd->ops->read) {
                kprintf("tty: keyboard device not available\n");
                kbd = NULL;
                return;
        }
        kprintf("Successful tty init\n");
}

static void tty_handle_char(char c)
{
        /* Newline ends the line */
        if (c == '\n') {
                if (tty_len < TTY_BUF_SIZE)
                        tty_buf[tty_len++] = c;
                line_ready = true;
                tty_putc(c);     /* echo */
                return;
        }

        /* Backspace */
        if (c == '\b') {
                if (tty_len > 0) {
                        tty_len--;
                        tty_putc('\b');
                        tty_putc(' ');
                        tty_putc('\b');
                }
                return;
        }

        /* Normal character */
        if (tty_len < TTY_BUF_SIZE - 1) {
                tty_buf[tty_len++] = c;
                tty_putc(c);     /* echo */
        }
}

size_t tty_read(void *buf, size_t n)
{
        char *out = buf;
        size_t i = 0;

        if (!kbd)
                return 0;

        /* Wait until a full line is available */
        while (!line_ready) {
                char c;
                kbd->ops->read(&c, 1);
                tty_handle_char(c);
        }

        /* Copy line to user buffer */
        while (i < n && i < tty_len) {
                out[i] = tty_buf[i];
                i++;
        }

        /* Reset buffer */
        tty_len = 0;
        line_ready = false;

        return i;
}


void tty_putc(char c)
{
        /* For now, forward directly to kernel console */
        kprintf("%c", c);
}
