global isr_pointer_table
extern isr_handler  ; C function: void isrHandler(int vector);

bits 64

isr_pointer_table:
%assign i 0
%rep 256
    dq isr_stub_%+i
%assign i i+1
%endrep

; Macro to generate stubs
%macro ISR_STUB 1
global isr_stub_%1
isr_stub_%1:
    mov rdi, %1        ; TODO: see if I need to PUSH, POP old RDI to preserve it
    mov rsi, rsp
    call isr_handler
    ; add rsp, 8         ; clean up argument
    iretq
%endmacro

; Generate 256 stubs
%assign i 0
%rep 256
    ISR_STUB i
%assign i i+1
%endrep
