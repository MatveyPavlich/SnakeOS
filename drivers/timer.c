#include "stdint.h"
#include "irq.h"
#include "pic.h"

// Implementation of the system timer

#define TIMER_IRQ       0                   // IRQ0 = PIT
#define TIMER_VECTOR    32                  // IDT vector after remap
#define CLOCK_FREQUENCY 100                 // Will give ~ 1 sec
#define VGA_MEMORY      ((uint8_t*)0xB8000)
#define VGA_COLS        80
#define VGA_ROWS        25

static uint64_t tick = 0;

/* Forward declarations */
static void timer_callback(int vector, struct interrupt_frame* frame);
static void timer_display_value(uint64_t* tick_pointer);

/* timer_init - initiate the system timer */
void timer_init()
{
        if (register_irq(TIMER_IRQ, timer_callback, NULL))
                kprint("Error: irq allocation to the timer failed\n");
        uint32_t divisor = 1193182 / CLOCK_FREQUENCY;

        // Command byte: channel 0, lowbyte/highbyte, mode 3 (square wave)
        outb(0x43, 0x36);
        outb(0x40, divisor & 0xFF);         // low byte
        outb(0x40, (divisor >> 8) & 0xFF);  // high byte
        pic_unmask_irq(TIMER_IRQ);
}

static void timer_callback(int vector, struct interrupt_frame* frame)
{
        (void)vector; (void)frame;
        tick++;

        /* TODO: implement set_clock_dirty_flag() to separate UI update 
         * from the interrupt handling code that should be fast */
        if (tick % CLOCK_FREQUENCY == 0) timer_display_value(&tick); 

        pic_send_eoi(TIMER_IRQ);
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
