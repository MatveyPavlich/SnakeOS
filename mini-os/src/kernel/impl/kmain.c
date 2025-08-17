// main.c
/*
    With -m64 gcc flag, function arguments are as follows:
    (1) RDI (2) RSI (3) RDX (4) RCX (5) R8 (6) R9

*/

#include "kprint.h"

extern void print_string_64(const char* str);

void kmain() {
    print_string_64("Hello from C!\0");
    kprint("This is my print!\0");
    while (1) __asm__("hlt");
}
