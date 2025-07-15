[org 0x7C00] ; bootloader offset
[bits 16]

main:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov si, MSG_REAL_MODE
    call print

    call check_a20
    call enable_a20_fast
    call check_a20
    hlt
    jmp $

check_a20:
    ; See if a20 is enabled
    xor ax, ax
    xor bx, bx
    mov si, 0x7DFE          ; Boot signature (0xAA55)
    mov di, 0x10FDE         ; 0xFFFF0 + 0x7DFE = 0x10FDE (1MB more)
    mov ax, [ds:si]         ; Read from 0x0000:0x7DFE = 0x07DFE
    mov bx, [es:si]         ; Read from 0xFFFF:0x7DFE = 0x10FDE
    cmp al, bl
    jne switch_to_pm
    ret

enable_a20_fast:
    in al, 0x92
    or al, 0x02
    out 0x92, al
    ret

switch_to_pm:
    cli                        ; Disable BIOS interrupts
    lgdt [gdt_descriptor]      ; Load the GDT descriptor
    mov eax, cr0
    or eax, 0x1                ; Set 32-bit mode bit in cr0
    mov cr0, eax
    jmp dword CODE_SEG:start_pm ; 4. far jump by using a different segment

%include "./src/utils/boot_sect_print.asm"
MSG_REAL_MODE db "Started in 16-bit real mode", 0xD, 0xA, 0x00

;============================ GDT ;============================
; don't remove labels, they're needed to compute sizes and jumps
gdt_start:

null_descriptor:
    dd 0x0 ; 4 byte
    dd 0x0 ; 4 byte
    
code_descriptor:
    dw 0xffff    ; First 16 bits of limit 
    dw 0x0       ; First 16 bits of the base
    db 0x0       ; Next 8 bits of the base
    db 10011010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + last 4 bits of limit
    db 0x0       ; segment base, bits 24-31
    
data_descriptor:
    dw 0xffff    ; First 16 bits of limit 
    dw 0x0       ; First 16 bits of the base
    db 0x0       ; Next 8 bits of the base
    db 10010010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + last 4 bits of limit
    db 0x0       ; segment base, bits 24-31

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start ; address (32 bit)

; define some constants for later use
CODE_SEG equ code_descriptor - gdt_start
DATA_SEG equ data_descriptor - gdt_start

; =================================================================
bits 32
start_pm:
    mov al, 'A'
    mov ah, 0x0C
    mov [0xb8000], ax

    jmp $

    ; mov ax, DATA_SEG ; 5. update the segment registers
    ; mov ds, ax
    ; mov ss, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax

    ; mov ebp, 0x90000 ; 6. update the stack right at the top of the free space
    ; mov esp, ebp

    ; call BEGIN_PM ; 7. Call a well-known label with useful code
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
