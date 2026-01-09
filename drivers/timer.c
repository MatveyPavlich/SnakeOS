// Implementation of the system timer
#include "cdev.h"
#include "interrupt.h"
#include "i8253.h"
#include "stddef.h"
#include "stdint.h"
#include "timer.h"
#include "util.h"

/* TODO: create a macro to later identify which value should be here */
#define CLOCK_FREQUENCY 100  /* 100 Hz (10 ms tick) */

static uint64_t tick = 0;
static timer_hook_t timer_secs_hook = NULL;

int timer_init()
{
        i8253_init(); /* TODO: later can add macro to detect which controller
                         is available */
        return 0;
}

/* Caller methods */
void timer_register_secs_hook(timer_hook_t hook)
{
        timer_secs_hook = hook;
}

uint64_t timer_get_ticks(void)
{
        return tick;
}

uint64_t timer_get_seconds(void)
{
        return (tick / CLOCK_FREQUENCY);
}

/* Ingestion API for timer chips to send ticks into timer core module. */
void timer_handle_tick()
{
        tick++;
        /* TODO: implement set_clock_dirty_flag() to separate UI update 
         * from the interrupt handling code that should be fast */
        if (tick % CLOCK_FREQUENCY == 0) {
                if (timer_secs_hook)
                        timer_secs_hook();
        }
}
