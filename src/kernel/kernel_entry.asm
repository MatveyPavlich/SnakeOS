org 0x90000
bits 32
; [extern main]

; call main
; jmp $

main:
    mov esi, MSG_KERNEL
    call print_string_pm
    hlt
    jmp $

%include "./src/bootloader/stage1/utils/32bit-print.asm"
MSG_KERNEL db "Kernel loaded", 0x00