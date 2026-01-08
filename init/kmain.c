#include "console.h"
#include "kprint.h"
#include "gdt.h"
#include "idt.h"
#include "i8259.h"
#include "init_ram.h"
#include "keyboard.h"
#include "timer.h"
#include "i8042.h"
#include "tests.h"

extern void print_64_bits(const char* str);


void kmain()
{
        /* Arch init */
        console_init();
        gdt_init();
        idt_init();
        irq_arch_init();
        kprint("Interrupts are enabled!!!\n");
        __asm__ volatile ("sti"); // Enable interrupts after PIC is set up

        /* Key drevices init */
        print_ram_map();
        timer_init();
        keyboard_init();

        /* Tests for debugging purposes */
        // tests_div_by_zero();

        while (1) __asm__("hlt");
}
