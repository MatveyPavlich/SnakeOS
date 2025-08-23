global isr_stub_table
extern isrHandler  ; C function: void isrHandler(int vector);

section .text

isr_stub_table:
%assign i 0
%rep 256
    dq isr_stub_%+i
%assign i i+1
%endrep

; Macro to generate stubs
%macro ISR_STUB 1
global isr_stub_%1
isr_stub_%1:
    push %1            ; push vector number (NASM understands this)
    call isrHandler
    add rsp, 8         ; clean up argument
    iretq
%endmacro

; Generate 256 stubs
%assign i 0
%rep 256
    ISR_STUB i
%assign i i+1
%endrep
