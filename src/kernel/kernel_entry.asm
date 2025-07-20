org 0x90000
bits 32
; [extern main]

; call main
; jmp $

main:
    mov esi, MSG_KERNEL
    call print_32_bits
    hlt
    jmp $

%include "./src/bootloader/stage1/print_32_bits.asm"
MSG_KERNEL db "Kernel loaded.", 0x00
