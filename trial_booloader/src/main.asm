[org 0x7C00] ; bootloader offset
[bits 16]

main:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov si, MSG_REAL_MODE
    call print ; This will be written after the BIOS messages

    ; call switch_to_pm
    jmp $ ; this will actually never be executed

%include "./src/utils/boot_sect_print.asm"
MSG_REAL_MODE db "Started in 16-bit real mode", 0xD, 0xA, 0x00

;============================ GDT ;============================
; gdt_start: ; don't remove the labels, they're needed to compute sizes and jumps
;     ; the GDT starts with a null 8-byte
;     dd 0x0 ; 4 byte
;     dd 0x0 ; 4 byte

; ; GDT for code segment. base = 0x00000000, length = 0xfffff
; ; for flags, refer to os-dev.pdf document, page 36
; gdt_code: 
;     dw 0xffff    ; segment length, bits 0-15
;     dw 0x0       ; segment base, bits 0-15
;     db 0x0       ; segment base, bits 16-23
;     db 10011010b ; flags (8 bits)
;     db 11001111b ; flags (4 bits) + segment length, bits 16-19
;     db 0x0       ; segment base, bits 24-31

; ; GDT for data segment. base and length identical to code segment
; ; some flags changed, again, refer to os-dev.pdf
; gdt_data:
;     dw 0xffff
;     dw 0x0
;     db 0x0
;     db 10010010b
;     db 11001111b
;     db 0x0

; gdt_end:

; ; GDT descriptor
; gdt_descriptor:
;     dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
;     dd gdt_start ; address (32 bit)

; ; define some constants for later use
; CODE_SEG equ gdt_code - gdt_start
; DATA_SEG equ gdt_data - gdt_start


; ;============================
; [bits 16]
; switch_to_pm:
;     cli ; 1. disable interrupts
;     lgdt [gdt_descriptor] ; 2. load the GDT descriptor
;     mov eax, cr0
;     or eax, 0x1 ; 3. set 32-bit mode bit in cr0
;     mov cr0, eax
;     jmp dword CODE_SEG:init_pm ; 4. far jump by using a different segment

; [bits 32]
; init_pm: ; we are now using 32-bit instructions
;     mov ax, DATA_SEG ; 5. update the segment registers
;     mov ds, ax
;     mov ss, ax
;     mov es, ax
;     mov fs, ax
;     mov gs, ax

;     mov ebp, 0x90000 ; 6. update the stack right at the top of the free space
;     mov esp, ebp

;     call BEGIN_PM ; 7. Call a well-known label with useful code
; ; =========================

; [bits 32]
; BEGIN_PM: ; after the switch we will get here
;     mov ebx, MSG_PROT_MODE
;     call print_string_pm ; Note that this will be written at the top left corner
;     jmp $

; MSG_PROT_MODE db "Loaded 32-bit protected mode", 0

; ;================================= Print
; [bits 32] ; using 32-bit protected mode

; ; this is how constants are defined
; VIDEO_MEMORY equ 0xb8000
; WHITE_ON_BLACK equ 0x0f ; the color byte for each character

; print_string_pm:
;     pusha
;     mov edx, VIDEO_MEMORY

; print_string_pm_loop:
;     mov al, [ebx] ; [ebx] is the address of our character
;     mov ah, WHITE_ON_BLACK

;     cmp al, 0 ; check if end of string
;     je print_string_pm_done

;     mov [edx], ax ; store character + attribute in video memory
;     add ebx, 1 ; next char
;     add edx, 2 ; next video memory position

;     jmp print_string_pm_loop

; print_string_pm_done:
;     popa
;     ret
; ;=================================

; bootsector
times 510-($-$$) db 0
dw 0xaa55
