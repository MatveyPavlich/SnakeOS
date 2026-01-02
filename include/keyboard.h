/* keyboard_init - initialise the keyboard module to allocate an IRQ line for
 *                 it
 */
int keyboard_init(void);

/* keyboard_handle_scancode - ingestion API for low-level drivers (i8042, USB)
 *                            to send scancodes to the keyboard module.
 * @scancode:                 scancode to be send.
 */
void keyboard_handle_scancode(uint8_t scancode);

