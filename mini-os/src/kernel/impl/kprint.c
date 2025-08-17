#include "stddef.h"
#include "stdint.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

static volatile uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static size_t cursor = 0;          // linear position 0..(80*25-1)
static uint8_t colour = 0x0A;      // red on black

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void kprint(const char* s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        char c = s[i];

        if (c == '\n') {
            cursor += VGA_WIDTH - (cursor % VGA_WIDTH);   // move to next line start
        } else {
            VGA_MEMORY[cursor++] = vga_entry(c, colour);
        }

        if (cursor >= VGA_WIDTH * VGA_HEIGHT) {
            // Simple wrap (or implement scrolling)
            cursor = 0;
        }
    }
}
