#pragma once
#include "stddef.h"

void tty_init(void);

/* Read up to n bytes from the tty (canonical mode) */
size_t tty_read(void *buf, size_t n);

/* Output a single character (used for echo / console) */
void tty_putc(char c);
