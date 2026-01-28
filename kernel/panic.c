#include "kprint.h"
#include "panic.h"

void panic(const char *msg)
{
        __asm__ volatile("cli");

        kprint("KERNEL PANIC: ");
        kprint(msg);
        kprint("\n");

        for (;;)
                __asm__ volatile("hlt");
}
