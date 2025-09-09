// main.c
/*
    With -m64 gcc flag, function arguments are as follows:
    (1) RDI (2) RSI (3) RDX (4) RCX (5) R8 (6) R9

*/

#include "kprint.h"
#include "gdt.h"
#include "idt.h"
#include "get_ram_info.h"

extern void print_64_bits(const char* str);

void kmain() {

    print_64_bits("Hello from C!\0");
    print_clear();
    kprintf("This is my string\n");
    kprintf("Hello world!\n");
    kprintf("Value: %d\n", 1234);
    kprintf("Hex: %x\n", 0xBEEF);
    kprintf("Char: %c\n", 'A');
    kprintf("String: %s\n", "OSDev!");

    gdtInit();
    idtInit();
    kprintf("Interrupts are enabled!!!\n");
    
    for (int i = 0; i < 20; i++) {
    kprintf("Entry %d: base=%llx length=%llx type=%u\n",
            i,
            ram_entries_by_bios[i].base,
            ram_entries_by_bios[i].length,
            ram_entries_by_bios[i].type);
    }
     
    // Division by zero interrup check
    // int a = 1, b = 0, c;
    // c = 1/0;

    kprintf("Is timer working?\n");
    while (1) __asm__("hlt");
}
