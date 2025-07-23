org 0x90000
bits 64

global kernel_entry
global print_string_64
extern main

kernel_entry:
    mov edi, 0xB8000                          ; Start of VGA text buffer
    mov ecx, 80 * 25                          ; Number of characters on screen
    mov ax, 0x0720                            ; ' ' (space) with gray-on-black attribute
    rep stosw                                 ; Fill ECX words (AX) into [EDI]

    mov esi, MSG_KERNEL
    call print_string_64
    ; call main
    hlt
    jmp $

; %include "./src/bootloader/stage1/64-bit-print.asm"
%include "./src/bootloader/stage1/utils_long_mode.asm"
; MSG_KERNEL db "Kernel loaded (long mode)", 0x00
MSG_KERNEL db "123456789-123456789-1234567890-123456789-123456789-123456789-123456789-123456789-123456789-123456789-", 0x00
