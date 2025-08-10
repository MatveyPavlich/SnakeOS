; Loaded at 0x90000
bits 64

global kernel_entry
global print_64_bits
extern kmain

PRINT_STRING_POSSITION dq 0                              ; Position for the next string                              

kernel_entry:
    
    ; Inform kerenel entry is entered
    mov [PRINT_STRING_POSSITION], rax                    ; Save line 
    xor eax, eax
    mov rdi, MSG_KERNEL
    call print_64_bits
    
    ; Jump into C
    call kmain
    
    ; Fall through to halt (should never reach it)
    hlt
    jmp $

%include "./src/bootloader/stage1/utils_long_mode.asm"
MSG_KERNEL db "SUCCESSFUL: Kernel entry loaded. Loading the kernel...", 0x00