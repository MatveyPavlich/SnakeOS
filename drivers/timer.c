// Implementation of the system timer
#include "stdint.h"
#include "stddef.h"
#include "interrupt.h"
#include "kprint.h"
#include "util.h"
#include "cdev.h"


#define TIMER_IRQ       0                   // IRQ0 = PIT
#define CLOCK_FREQUENCY 100                 // Will give ~ 1 sec
#define PIT_BASE_FREQ   1193182
#define VGA_MEMORY      ((uint8_t*)0xB8000)
#define VGA_COLS        80
#define VGA_ROWS        25

/* Forward declarations */
static void timer_callback(int irq, struct interrupt_frame* frame, void *dev);
static void timer_display_value(uint64_t* tick_pointer);
size_t timer_read_ticks(void *buf, size_t n);

static uint64_t tick = 0;
static struct cdev timer;
static struct cdev_ops timer_ops = {
        .read = &timer_read_ticks,
        .write = NULL,
        .ioctl = NULL,
};

/* timer_init - initiate the system timer */
int timer_init()
{
        if (irq_request(TIMER_IRQ, timer_callback, NULL)) {
                kprintf("Error: irq allocation to the timer failed\n");
                return 1;
        }
        uint32_t divisor = PIT_BASE_FREQ / CLOCK_FREQUENCY;

        // Command byte: channel 0, lowbyte/highbyte, mode 3 (square wave)
        outb(0x43, 0x36);
        outb(0x40, divisor & 0xFF);         // low byte
        outb(0x40, (divisor >> 8) & 0xFF);  // high byte

        timer = (struct cdev) {
                .name = "timer",
                .ops  = &timer_ops,
                .priv = NULL,
        };
        if (cdev_register(&timer)) {
                kprintf("Error: cdev registration for timer failed\n");
                return 1;
        }
        return 0;
}

static void timer_callback(int irq, struct interrupt_frame* frame, void *dev)
{
        (void)irq; (void)frame, (void)dev;
        tick++;
        /* TODO: implement set_clock_dirty_flag() to separate UI update 
         * from the interrupt handling code that should be fast */
        if (tick % CLOCK_FREQUENCY == 0)
                timer_display_value(&tick); 
}

/* Caller API for finding the number of ticks. Handle the data race in the
 * future to prevent a caller receiving a value while a new PIT interrupt
 * is executing
 */
size_t timer_read_ticks(void *buf, size_t n)
{
        (void)n;
        memset(buf, tick, 8);
        return 8; /* Ticks is 64 bits */ 
}

static void timer_display_value(uint64_t* tick_pointer)
{
        uint64_t total_seconds = *tick_pointer / CLOCK_FREQUENCY;
        uint64_t hours = total_seconds / 3600;
        uint64_t minutes = (total_seconds / 60) % 60;
        uint64_t seconds = total_seconds % 60;

        // Format hh:mm:ss
        char buf[9];
        buf[0] = '0' + (hours / 10) % 10;
        buf[1] = '0' + (hours % 10);
        buf[2] = ':';
        buf[3] = '0' + (minutes / 10);
        buf[4] = '0' + (minutes % 10);
        buf[5] = ':';
        buf[6] = '0' + (seconds / 10);
        buf[7] = '0' + (seconds % 10);
        buf[8] = '\0';

        // Position: last row, right-aligned
        int row = VGA_ROWS - 1;
        int col = VGA_COLS - 8;  // 8 chars wide
        int offset = (row * VGA_COLS + col) * 2;

        for (int i = 0; i < 8; i++) {
                VGA_MEMORY[offset + i*2] = buf[i];      // character
                VGA_MEMORY[offset + i*2 + 1] = 0x0F;    // white on black
        }
}
