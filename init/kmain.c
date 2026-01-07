#include "kprint.h"
#include "gdt.h"
#include "idt.h"
#include "i8259.h"
#include "init_ram.h"
#include "keyboard.h"
#include "tty.h"
#include "timer.h"
#include "i8042.h"
#include "tests.h"

extern void print_64_bits(const char* str);

void kernel_console(void)
{
        char buf[128];

        while (1) {
                size_t n = tty_read(buf, sizeof(buf));
                buf[n] = '\0';

                kprint("got: %s\n", buf);
        }
}

void kmain()
{
        print_clear();

        /* Arch init */
        gdt_init();
        idt_init();
        irq_arch_init();
        kprint("Interrupts are enabled!!!\n");
        __asm__ volatile ("sti"); // Enable interrupts after PIC is set up

        /* Key drevices init */
        print_ram_map();
        timer_init();
        keyboard_init();
        tty_init();

        kernel_console();

        /* Tests for debugging purposes */
        // tests_div_by_zero();

        while (1) __asm__("hlt");
}
