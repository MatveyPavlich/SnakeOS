global idt_stub_table     ; Expose the stub table to load it into idt
global idt_load           ; Expose idt_load function to idt.c
extern exception_handle   ; Function from exception.c to handle CPU exceptions.
extern irq_handle         ; Function from irq.h to handle the interrupt.
                          ; Not sure if I am supposed to directly hardcode
                          ; it into IDT though (arch specific).  Lets see if
                          ; I'll regret it later!

%define PIC_IDT_BASE 0x20 ; TODO: Will need a macro if will support APIC

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
        ; rSP at entry points to CPU-pushed frame
        mov rsi, rsp            ; rsi = interrupt_frame*

        push rdi                ; preserve caller-saved regs we touch
        push rax

        mov rdi, %1             ; rdi = vector number

        cmp rdi, PIC_IDT_BASE
        jl .exception

        ; -------- IRQ path --------
        sub rdi, PIC_IDT_BASE   ; convert vector → IRQ
        call irq_handle
        jmp .done

.exception:
        ; -------- exception path --------
        call exception_handle

.done:
        pop rax
        pop rdi
        iretq
%endmacro

; Generate 256 stubs
%assign i 0
%rep 256
        STUB i
%assign i i+1
%endrep
