// API interface for the clock
void timer_init();

/* Caller methods to get the system time */
uint64_t timer_get_ticks(void);
uint64_t timer_get_seconds(void);
// void timer_register_tick_hook(void (*fn)(void));

/* Ingestion API for the clock subsystem. To be used by timer controllers only
 */
void timer_handle_tick();
