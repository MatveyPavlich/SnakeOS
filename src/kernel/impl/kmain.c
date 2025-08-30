// main.c
/*
    With -m64 gcc flag, function arguments are as follows:
    (1) RDI (2) RSI (3) RDX (4) RCX (5) R8 (6) R9

*/

#include "kprint.h"
#include "gdt.h"
#include "idt.h"
// #include "timer.h"

extern void print_64_bits(const char* str);
// extern void initTimer(uint32_t freq);

void kmain() {

    print_64_bits("Hello from C!\0");
    print_clear();
    kprint("This is my string\n");
    gdtInit();
    idtInit();
    __asm__ volatile ("sti");
    kprint("Interrupts are enabled!!!\n");
    
    // Division by zero interrup check
    // int a = 1, b = 0, c;
    // c = 1/0;

    // initTimer(100);
    // kprint("Is timer working?\n");
    while (1) __asm__("hlt");
}
