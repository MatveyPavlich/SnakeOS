org 0x90000
bits 64
extern main

main:
    mov edi, 0xB8000                          ; Start of VGA text buffer
    mov ecx, 80 * 25                          ; Number of characters on screen
    mov ax, 0x0720                            ; ' ' (space) with gray-on-black attribute
    rep stosw                                 ; Fill ECX words (AX) into [EDI]

    mov esi, MSG_KERNEL
    call print_string_64
    ; call main
    hlt
    jmp $

%include "./src/bootloader/stage1/64-bit-print.asm"
MSG_KERNEL db "Kernel loaded (long mode)", 0x00
