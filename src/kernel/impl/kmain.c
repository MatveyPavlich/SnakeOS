// main.c
/*
    With -m64 gcc flag, function arguments are as follows:
    (1) RDI (2) RSI (3) RDX (4) RCX (5) R8 (6) R9

*/

#include "kprint.h"
#include "gdt.h"
#include "idt.h"
#include "init_ram.h"

extern void print_64_bits(const char* str);

void shell()
{
    while(1) {
        kprintf("SnakeOS> ");
        
        char c;
        do {c = get_char();} while (c != '\n');
        kprintf('\n');   
    }
}

void kmain()
{
    print_64_bits("Hello from C!\0");
    print_clear();
    kprintf("This is my string\n");
    kprintf("Hello world!\n");

    gdtInit();
    idtInit();
    kprintf("Interrupts are enabled!!!\n");

    print_ram_map();
    shell();
        
    // Division by zero interrup check
    // int a = 1, b = 0, c;
    // c = 1/0;

    while (1) __asm__("hlt");
}
