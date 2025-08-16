; Loaded at 0x90000
bits 64
org 0x90000

; global kernel_entry
; global print_string_64
; extern kmain
                          

kernel_entry:
    
    ; Inform kerenel entry is entered
    mov rsi, MSG_KERNEL
    call print_string_64
    
    ; Jump into C
    ; jmp kmain                                            ; Don't use CALL cause it will save return address on the stack => stack will start in the different place for kmain
    
    ; Fall through to halt (should never reach it)
    hlt
    jmp $

%include "./src/bootloader/utils/64-bit-print.asm"
MSG_KERNEL db "SUCCESSFUL: Kernel entry loaded. Loading the kernel...", 0x00