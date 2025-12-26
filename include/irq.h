#pragma once
#include "stdint.h"

// API interface for irq module

struct interrupt_frame {
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t error_code;
};

typedef void (*irq_handler_t)(struct interrupt_frame *);
int register_irq(int irq, irq_handler_t handler, void *dev);
void dispatch_irq(int irq);

// typedef void (*isrptr_t)(int vector, struct interrupt_frame* frame);
// void isrHandler(int index, struct interrupt_frame* frame);
// void register_interrupt_handler(int vector, isrptr_t handler);
// void init_timer(uint32_t frequency);
// void gp_fault_handler(int vector, struct interrupt_frame* frame);


