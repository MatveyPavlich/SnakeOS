// isr.h
#pragma once
#include "stdint.h"

struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t error_code;
};

typedef void (*isrptr_t)(int vector, struct interrupt_frame* frame);
void isrHandler(int index, struct interrupt_frame* frame);
void register_interrupt_handler(int vector, isrptr_t handler);
void init_timer(uint32_t frequency);
void gp_fault_handler(int vector, struct interrupt_frame* frame);
