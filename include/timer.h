// API interface for the clock
void timer_init();

/* Read number of ticks (systimer). Will always return 8 bytes, so 'n' is not
 * really used
 */
size_t timer_read_ticks(void *buf, size_t n);
