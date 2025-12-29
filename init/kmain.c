#include "kprint.h"
#include "gdt.h"
#include "idt.h"
#include "i8259.h"
#include "init_ram.h"
#include "keyboard.h"
#include "timer.h"
#include "tests.h"

extern void print_64_bits(const char* str);
extern char get_char(void);

void kmain()
{
        print_64_bits("Hello from C!\0");
        print_clear();
        kprintf("This is my string\n");
        kprintf("Hello world!\n");

        gdt_init();
        idt_init();
        irq_arch_init();
        kprintf("Interrupts are enabled!!!\n");
        __asm__ volatile ("sti"); // Enable interrupts after PIC is set up
        print_ram_map();
        timer_init();
        keyboard_init();

        // tests_div_by_zero();

        while (1) __asm__("hlt");
}
