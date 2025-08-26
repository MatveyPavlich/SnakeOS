// isr.h
#pragma once
#include "stdint.h"

typedef void (*isrptr_t)(int index);
void isrHandler(int index);
void register_interrupt_handler(int vector, isrptr_t handler);