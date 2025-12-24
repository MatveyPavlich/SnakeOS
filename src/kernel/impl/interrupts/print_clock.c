#include "stdint.h"

#define VGA_MEMORY ((uint8_t*)0xB8000)
#define VGA_COLS 80
#define VGA_ROWS 25

static const int PIT_FREQUENCY = 100;

void print_clock(uint64_t* tick_pointer)
{
        uint64_t total_seconds = *tick_pointer / PIT_FREQUENCY;
        uint64_t hours = total_seconds / 3600;
        uint64_t minutes = (total_seconds / 60) % 60;
        uint64_t seconds = total_seconds % 60;

        // Format hh:mm:ss
        char buf[9];
        buf[0] = '0' + (hours / 10) % 10;
        buf[1] = '0' + (hours % 10);
        buf[2] = ':';
        buf[3] = '0' + (minutes / 10);
        buf[4] = '0' + (minutes % 10);
        buf[5] = ':';
        buf[6] = '0' + (seconds / 10);
        buf[7] = '0' + (seconds % 10);
        buf[8] = '\0';

        // Position: last row, right-aligned
        int row = VGA_ROWS - 1;
        int col = VGA_COLS - 8;  // 8 chars wide
        int offset = (row * VGA_COLS + col) * 2;

        for (int i = 0; i < 8; i++) {
                VGA_MEMORY[offset + i*2] = buf[i];      // character
                VGA_MEMORY[offset + i*2 + 1] = 0x0F;    // white on black
        }
}
