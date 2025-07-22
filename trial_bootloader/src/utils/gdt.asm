;============================ GDT ;============================
; Don't remove labels, they're needed to compute sizes and jumps
; Add an entry for the 64-bit mode
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

long_mode_code_descriptor:
    dw 0x0000          ; Limit (ignored)
    dw 0x0000          ; Base[15:0]
    db 0x00            ; Base[23:16]
    db 10011010b       ; Access: exec/read, present
    db 00100000b       ; Flags: L=1, D=0
    db 0x00            ; Base[31:24]

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start               ; address (32 bit)

; define some constants for later use
CODE_SEG equ code_descriptor - gdt_start
DATA_SEG equ data_descriptor - gdt_start
LONG_CODE_SEG equ long_mode_code_descriptor - gdt_start