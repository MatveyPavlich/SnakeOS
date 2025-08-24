// isr.h
#pragma once
#include "stdint.h"

typedef void (*isr_t)(int vector);

void register_interrupt_handler(int n, isr_t handler);
