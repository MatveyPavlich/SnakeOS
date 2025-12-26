/* API interface for the i8259A controller (PIC) */

struct irq_chip {
	const char	*name;
	unsigned int	(*irq_startup)(struct irq_data *data);
	void		(*irq_shutdown)(struct irq_data *data);
	void		(*irq_enable)(struct irq_data *data);
	void		(*irq_disable)(struct irq_data *data);
}
void pic_init(void)
void pic_unmask_irq(int irq)
void pic_send_eoi(int irq)
