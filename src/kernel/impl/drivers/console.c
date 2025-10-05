/*
===============================================================================
MODULE OVERVIEW
===============================================================================

1. kprintf.c
   - Implements `kprintf()` and internal `vkprintf()` using `vsnprintf`.
   - Handles string formatting, numbers, and specifiers.
   - Sends formatted characters to `console_putc()`.

2. console.c
   - Provides terminal-like abstraction.
   - Maintains cursor (row, col).
   - Handles special characters:
        '\n'   → new line
        '\b'   → delete previous character
        '\t'   → move cursor by tab width
   - Implements scrolling when cursor passes bottom of screen.
   - For each visible character, calls `vga_putc(row, col, c)`.

3. vga.c
   - VGA text driver that writes directly to 0xB8000.
   - Handles:
        - Setting a character with colour attribute.
        - Clearing the screen.
        - Moving the hardware cursor (via I/O ports 0x3D4, 0x3D5).
   - No knowledge of cursor logic or formatting.

===============================================================================
FLOW OF EXECUTION
===============================================================================

    kprintf("Hello\n"); -> console_putc('H') -> vga_putc(0,0,'H') -> 0xB8000

===============================================================================
*/
