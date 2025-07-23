org 0x90000
bits 64

global kernel_entry
global print_string_64
extern main

kernel_entry:
    mov [PRINT_STRING_POSSITION], rax                    ; Save line 
    mov esi, MSG_KERNEL
    call print_string_64
    ; call main
    hlt
    jmp $

%include "./src/bootloader/stage1/utils_long_mode.asm"
MSG_KERNEL db "SUCCESSFUL: Kernel entry loaded. Loading the kernel...", 0x00
PRINT_STRING_POSSITION dw 0