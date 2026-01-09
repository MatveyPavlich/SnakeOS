// API interface for the clock
#include "stdint.h"

int timer_init();

typedef void (*timer_hook_t)(void);

/* Caller methods to get the system time */
uint64_t timer_get_ticks(void);
uint64_t timer_get_seconds(void);
void timer_register_secs_hook(timer_hook_t hook);

/* Ingestion API for the clock subsystem. To be used by timer controllers only
 */
void timer_handle_tick();
