// TODO: Implement here for now, but later should be in exceptions.c
// static void gp_fault_handler(int vector, struct interrupt_frame* frame)
// {
//         kprintf("General Protection Fault: %d\n", vector);
//         kprintf("RIP = %x\n",frame->rip);
//         kprintf("System halted\n");
//
//         while (1) __asm__("hlt");
// }
