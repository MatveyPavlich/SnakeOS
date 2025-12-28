// API interface for the exceptions subsystem

#include "interrupt.h"

void exceptions_handle(int vector, struct interrupt_frame *frame);
