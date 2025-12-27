global idt_stub_table    ; Expose the stub table to load it into idt
extern irq_handle_vector ; Function from irq.h to handle the interrupt.
                         ; Not sure if I am supposed to directly hardcode
                         ; it into IDT though (arch specific).  Lets see if
                         ; I'll regret it later!

bits 64

; idt_load(struct idt_metadata *m) - Function to load IDT into the CPU
idt_load:
        lidt [rdi]   ; pointer to idt_metadata passed in RDI
        ret

; Generate a stub table for idt and fill it with pointers to irq_handle_vector
; in irq.h
idt_stub_table:
%assign i 0
%rep 256
        dq stub_%+i
%assign i i+1
%endrep

; Macro to generate stubs
%macro STUB 1
global stub_%1
stub_%1:
        mov rdi, %1 ; TODO: see if I need to PUSH, POP old RDI to preserve it
        mov rsi, rsp
        call irq_handle_vector
        ; add rsp, 8         ; clean up argument
        iretq
%endmacro

; Generate 256 stubs
%assign i 0
%rep 256
        STUB i
%assign i i+1
%endrep
