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

void print_ram_map(void) {
    for (int i = 0; i < 10; i++) {
        struct ram_entry e = ram_entries_by_bios[i];
        kprintf("Entry %d:", i);
        kprintf(" Base = %x", (unsigned int)(e.base & 0xFFFFFFFF));
        kprintf(" Length = %x", (unsigned int)(e.length & 0xFFFFFFFF));
        kprintf(" Type = %d\n", (int)e.type);
    }
}

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
    
    print_ram_map();
     
    // Division by zero interrup check
    // int a = 1, b = 0, c;
    // c = 1/0;

    kprintf("Is timer working?\n");
    while (1) __asm__("hlt");
}
