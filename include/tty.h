/* tty API interface */

#include "input.h"

void tty_init(void);
void tty_handle_key(const struct key_event *ev);
