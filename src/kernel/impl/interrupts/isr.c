#include "kprint.h"

void isrHandler(int vector) {
    kprint("Interrupt: ");
    // simple decimal print
    char buf[4];
    buf[0] = '0' + (vector / 10);
    buf[1] = '0' + (vector % 10);
    buf[2] = '\n';
    buf[3] = 0;
    kprint(buf);

    // If it's IRQ, you must send EOI to PIC/APIC here
}
