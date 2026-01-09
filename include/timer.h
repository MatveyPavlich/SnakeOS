// API interface for the clock
#include "stdint.h"

/* timer_init - initiate the system timer */
int timer_init();

/* timer_get_ticks - Return the current raw timer tick count. */
uint64_t timer_get_ticks(void);

/* timer_get_seconds - Return the n. of seconds since timer
 *                     subsystem initialisation.
 */
uint64_t timer_get_seconds(void);

/* Callback type for the caller to follow for the timer subsystem to execute */
typedef void (*timer_hook_t)(void);
/* timer_register_secs_hook - Register a per-second timer callback.
 * @hook:                     Function to be invoked once per second.
 */
void timer_register_secs_hook(timer_hook_t hook);

/* Ingestion API for the clock subsystem. To be used by timer controllers only
 */
void timer_handle_tick();
