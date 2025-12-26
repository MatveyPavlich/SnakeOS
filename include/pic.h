/* API interface for the i8259 controller (PIC) */

void pic_init(void)
void pic_unmask_irq(int irq)
void pic_send_eoi(int irq)
