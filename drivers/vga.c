// vga.c - dumb pixel pusher

#include "stdint.h"
#include "stddef.h"

#define VGA_NUM_COLS   80
#define VGA_NUM_ROWS   25
#define VGA_MEMO_START 0xB8000

#define VGA_COLOR(fg, bg) ((bg << 4) | (fg & 0x0F))

enum {
        VGA_COLOUR_BLACK = 0,
        VGA_COLOUR_WHITE = 15,
        VGA_COLOUR_RED = 2
};

typedef struct {
        uint8_t character;
        uint8_t colour;
} __attribute__((packed)) vga_char_t;

static vga_char_t* const vga_memory = (vga_char_t*)VGA_MEMO_START;
static uint8_t default_colour = VGA_COLOR(VGA_COLOUR_BLACK, VGA_COLOUR_WHITE);

void vga_put_char(size_t row, size_t col, char c)
{
        if (row >= VGA_NUM_ROWS || col >= VGA_NUM_COLS)
                return;

        vga_char_t ch = { c, default_colour };
        vga_memory[col + VGA_NUM_COLS * row] = ch;
}

void vga_clear_row(size_t row)
{
        for (size_t col = 0; col < VGA_NUM_COLS; col++)
                vga_put_char(row, col, ' ');
}

void vga_clear_screen(void)
{
        for (size_t row = 0; row < VGA_NUM_ROWS; row++) {
                for (size_t col = 0; col < VGA_NUM_COLS; col++)
                        vga_put_char(row, col, ' ');
        }
}

/* TODO: Implement scroll */
// void vga_scroll_up(void)
