/* Driver for the Programmable Interval Timer (PIT) */

#include "interrupt.h"

/ * Hardware ports */
#define PIT_CH0_DATA         0x40
#define PIT_CH1_DATA         0x41
#define PIT_CH2_DATA         0x42
#define PIT_CMD              0x43

/* Access mode */
#define PIT_ACCESS_LATCH     0x00
#define PIT_ACCESS_LOBYTE    0x10
#define PIT_ACCESS_HIBYTE    0x20
#define PIT_ACCESS_LOHI      0x30

/* Operating mode */
#define PIT_MODE_0           0x00  // interrupt on terminal count
#define PIT_MODE_2           0x04  // rate generator
#define PIT_MODE_3           0x06  // square wave

/* Binary */
#define PIT_BINARY           0x00
#define PIT_BCD              0x01

/* Channel select */
#define PIT_CH0              0x00
#define PIT_CH1              0x40
#define PIT_CH2              0x80

int i8253_init(void)
{
        pit_set_mode(PIT_CH0, PIT_MODE_3);
        return 1;
}

static inline void pit_set_mode(uint8_t channel, uint8_t mode)
{
        outb(PIT_CMD, channel | PIT_ACCESS_LOHI | mode | PIT_BINARY);
}

static void i8253_irq_handle(int irq, struct interrupt_frame* frame, void *dev)
{
        // Call the time core module
}

