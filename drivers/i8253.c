/* Driver for the Programmable Interval Timer (PIT) */

#include "interrupt.h"
#include "util.h"
#include "timer.h"
#include "kprint.h"

/* Hardware ports */
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

/* Timer stuff */
#define TIMER_IRQ       0    // IRQ0 = PIT
#define CLOCK_FREQUENCY 100  // Will give ~ 1 sec
#define PIT_BASE_FREQ   1193182

/* Forward declarations */
static inline void pit_set_mode(uint8_t channel, uint8_t mode);
static void i8253_irq_handle(int irq, struct interrupt_frame* frame, void *dev);

int i8253_init(void)
{
        /* Set up PIT to the correct frequency with the square mode*/
        uint32_t divisor = PIT_BASE_FREQ / CLOCK_FREQUENCY;
        pit_set_mode(PIT_CH0, PIT_MODE_3);
        outb(PIT_CH0_DATA, divisor & 0xFF);
        outb(PIT_CH0_DATA, (divisor >> 8) & 0xFF);

        if (irq_request(TIMER_IRQ, i8253_irq_handle, NULL)) {
                kprintf("Error: irq allocation for PIT failed\n");
                return 1;
        }

        return 0;
}

static inline void pit_set_mode(uint8_t channel, uint8_t mode)
{
        outb(PIT_CMD, channel | PIT_ACCESS_LOHI | mode | PIT_BINARY);
}

static void i8253_irq_handle(int irq, struct interrupt_frame* frame, void *dev)
{
        (void)irq; (void)frame, (void)dev;
        timer_handle_tick();
}
