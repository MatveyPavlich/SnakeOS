#include "stddef.h"

void vga_clear_row(size_t row);
void vga_put_char(size_t row, size_t col, char c);
void vga_clear_screen(void);
// void vga_scroll_up(void);
