; Loaded at 0x90000
; org 0x90000 ; How come do I not need this?
bits 64

global kernel_entry
global print_string_64
extern kmain
                          

kernel_entry:
    
    ; Inform kerenel entry is entered
    mov rdi, MSG_KERNEL
    call print_string_64
    
    ; Jump into C
    jmp kmain                                            ; Don't use CALL cause it will save return address on the stack => stack will start in the different place for kmain
    
    ; Fall through to halt (should never reach it)
    hlt
    jmp $

%include "./src/bootloader/utils/64-bit-print.asm"
MSG_KERNEL db "Kernel entry loaded.", 0x00