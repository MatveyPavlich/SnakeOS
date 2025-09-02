// print.c
#include "stdint.h"
#include "stddef.h"
#include "stdarg.h"

enum {
    PRINT_COLOUR_BLACK = 0,
    PRINT_COLOUR_WHITE = 15,
};

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

typedef struct {
    uint8_t character;
    uint8_t colour;
} Char;

Char* buffer = (Char*) 0xb8000;
size_t col = 0;
size_t row = 0;
uint8_t colour = PRINT_COLOUR_WHITE | PRINT_COLOUR_BLACK << 4;

void clear_row(size_t row) {
    Char empty = (Char) {
        .character = ' ',
        .colour = colour,
    };

    for (size_t col = 0; col < NUM_COLS; col++) {
        buffer[col + NUM_COLS * row] = empty;
    }
}

void print_clear() {
    for (size_t i = 0; i < NUM_ROWS; i++) {
        clear_row(i);
    }
}

void print_newline() {
    col = 0;

    if (row < NUM_ROWS - 1) {
        row++;
        return;
    }

    // scroll screen up
    for (size_t r = 1; r < NUM_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            buffer[c + NUM_COLS * (r - 1)] = buffer[c + NUM_COLS * r];
        }
    }

    clear_row(NUM_ROWS - 1);
}

void print_char(char character) {
    if (character == '\n') {
        print_newline();
        return;
    }

    if (col >= NUM_COLS) {
        print_newline();
    }

    buffer[col + NUM_COLS * row] = (Char) {
        .character = (uint8_t) character,
        .colour = colour,
    };

    col++;
}

// -------------------- Helpers -------------------- //
void print_str(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

// Convert integer to string (decimal)
void itoa_dec(int value, char* buffer) {
    char temp[32];
    int i = 0;
    int neg = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0) {
        neg = 1;
        value = -value;
    }

    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (neg) temp[i++] = '-';

    // reverse
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

// Convert integer to string (hex)
void itoa_hex(unsigned int value, char* buffer) {
    const char* digits = "0123456789ABCDEF";
    char temp[32];
    int i = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = digits[value % 16];
        value /= 16;
    }

    buffer[0] = '0';
    buffer[1] = 'x';

    int j = 2;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

// -------------------- Kernel Printf -------------------- //
void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == 'd') {
                int val = va_arg(args, int);
                char buf[32];
                itoa_dec(val, buf);
                print_str(buf);
            } else if (fmt[i] == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                char buf[32];
                itoa_hex(val, buf);
                print_str(buf);
            } else if (fmt[i] == 's') {
                char* s = va_arg(args, char*);
                print_str(s);
            } else if (fmt[i] == 'c') {
                char c = (char) va_arg(args, int);
                print_char(c);
            } else {
                print_char('%');
                print_char(fmt[i]);
            }
        } else {
            print_char(fmt[i]);
        }
    }

    va_end(args);
}
