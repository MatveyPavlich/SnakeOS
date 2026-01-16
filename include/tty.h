/* tty API interface */

#include "input.h"

/* Subsystem initialisation */
void tty_init(void);

/* Ingestion API for the tty subsystem */
void tty_handle_key(const struct key_event *ev);
