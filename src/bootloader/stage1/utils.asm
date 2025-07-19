;=====================================| Ensure A20 is enabled |===================================== 
ensure_a20:
    call check_a20
    jne .done                                 ; Already enabled

    call enable_a20_fast
    call check_a20
    jne .done                                 ; Successfully enabled

    jmp halt_with_failure                     ; Still not working
.done:
    ret

check_a20:
    push es
    push ds

    ; Set up memory segments
    cli
    mov ax, 0x0000
    mov ds, ax
    mov di, 0x0500 
    not ax                                    ; Make ax = 0xFFFF (0xFFFF:0x0510 would map to 0x0000:0x0500 if a20 disabled)
    mov es, ax
    mov si, 0x0510

    ; Save original values
    mov al, [ds:di]
    mov ah, [es:si]

    ; Write test values
    mov byte [ds:di], 0xBB
    mov byte [es:si], 0xFF

    ; Compare
    mov bl, [ds:di]
    mov cl, [es:si]
    cmp bl, cl

    ; Restore originals
    mov [ds:di], al
    mov [es:si], ah
    pop ds
    pop es
    sti                                       ; ZF != 0 if different (A20 enabled)
    ret

; -------------------------
enable_a20_fast:
    in al, 0x92
    or al, 0x02
    out 0x92, al                              ; enable a20 via fast method
    ret

halt_with_failure:
    mov si, A20_FAILED
    call print
    hlt
    jmp $

A20_FAILED:    db "A20 couldn't be enabled. System halted", 0xD, 0xA, 0x00





;============================ GDT ;============================
; don't remove labels, they're needed to compute sizes and jumps
gdt_start:

null_descriptor:
    dd 0x0                     ; 4 byte
    dd 0x0                     ; 4 byte
    
code_descriptor:
    dw 0xffff                  ; First 16 bits of limit 
    dw 0x0                     ; First 16 bits of the base
    db 0x0                     ; Next 8 bits of the base
    db 10011010b               ; flags (8 bits)
    db 11001111b               ; flags (4 bits) + last 4 bits of limit
    db 0x0                     ; segment base, bits 24-31

data_descriptor:
    dw 0xffff                  ; First 16 bits of limit 
    dw 0x0                     ; First 16 bits of the base
    db 0x0                     ; Next 8 bits of the base
    db 10010010b               ; flags (8 bits)
    db 11001111b               ; flags (4 bits) + last 4 bits of limit
    db 0x0                     ; segment base, bits 24-31

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start               ; address (32 bit)

; define some constants for later use
CODE_SEG equ code_descriptor - gdt_start
DATA_SEG equ data_descriptor - gdt_start