// main.c
/*
    With -m64 gcc flag, function arguments are as follows:
    (1) RDI (2) RSI (3) RDX (4) RCX (5) R8 (6) R9

*/

#include "kprint.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"

extern void print_64_bits(const char* str);
extern void init_timer(uint32_t freq);

void kmain() {
    print_64_bits("Hello from C!\0");
    print_clear();
    kprint("This is my string\n");
    gdtInit();
    idtInit();
    pic_init();
    kprint("This is my second string!!!\n");
    init_timer(100);
    __asm__ volatile ("sti");
    kprint("Is timer working?\n");
    while (1) __asm__("hlt");
}
