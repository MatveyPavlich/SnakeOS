;=====================================| Ensure A20 is enabled |===================================== 
ensure_a20:
    
    ; Check if A20 line is already enabled
    call .check_a20
    jne .done                                  ; Already enabled

    ; Enable A20 using fast method (not keyboard controller)
    call .enable_a20_fast
    call .check_a20
    jne .done                                  ; Successfully enabled

    ; Halt the system if A20 was not enabled
    jmp .halt_with_failure

.check_a20:
    ; Presever current es and ds segments
    push es
    push ds

    ; Set up memory segments for the test
    cli                                       ; Disable interrupts
    mov ax, 0x0000                            ; Case 1: 0x0000:0x0500 = 0x00500 (ds:di)
    mov ds, ax                                ; Case 1 set up
    mov di, 0x0500                            ; Case 1 set up
    not ax                                    ; Make ax = 0xFFFF
    mov es, ax                                ; Case 2: 0xFFFF:0x0510 = 0x100500 = 0x00500 if a20 disabled (es:ei)
    mov si, 0x0510                            ; Case 2 set up

    ; Save original values
    mov al, [ds:di]                           ; Preserve original value at case 1 (ds:di)
    mov ah, [es:si]                           ; Preserve original value at case 1 (es:si)

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

.enable_a20_fast:
    in al, 0x92
    or al, 0x02
    out 0x92, al                              ; enable a20 via fast method
    ret

.done:
    ret

.halt_with_failure:
    mov si, A20_FAILED
    call print                                ; SI = string pointer; print is from utils_shared.asm
    call halt                                 ; Input = void; halt is from utils_shared.asm.

A20_FAILED:    db "A20 couldn't be enabled. System halted", 0xD, 0xA, 0x00





; ===========================| GDT |============================
; - Don't remove labels, they're needed to compute sizes and jumps
; - Each label is a 8 byte row in the GDB table
; ==============================================================
gdt_start:

    ; First GDT record (bytes 0-7)
    .null_descriptor:
        dd 0x0                                ; 4 byte
        dd 0x0                                ; 4 byte
    
    ; Code segment (bytes 8-15): BASE = 0x00000000 LIMIT = 0xfffff * 0x1000 (4KiB granularity)
    ; = 0xFFFFF000 (last 4KiB page) => 0xFFFFF000 + 0xFFF = 0xFFFFFFFF (i.e. covering 4 GB)
    .code_descriptor:
        dw 0xffff                             ; First 16 bits of limit 
        dw 0x0                                ; First 16 bits of the base
        db 0x0                                ; Next 8 bits of the base
        db 10011010b                          ; flags (8 bits)
        db 11001111b                          ; flags (4 bits) + last 4 bits of limit
        db 0x0                                ; segment base, bits 24-31

    ; Code segment (bytes 8-15): BASE = 0x00000000 LIMIT = 0xfffff * 0x1000 (4KiB granularity)
    ; = 0xFFFFF000 (last 4KiB page) => 0xFFFFF000 + 0xFFF = 0xFFFFFFFF (i.e. covering 4 GB)
    .data_descriptor:
        dw 0xffff                             ; First 16 bits of limit 
        dw 0x0                                ; First 16 bits of the base
        db 0x0                                ; Next 8 bits of the base
        db 10010010b                          ; flags (8 bits)
        db 11001111b                          ; flags (4 bits) + last 4 bits of limit
        db 0x0                                ; segment base, bits 24-31

    ; LIMIT = BASE = 0 since ignored
    .long_mode_code_descriptor:
        dw 0x0000                             ; Limit (ignored)
        dw 0x0000                             ; Base[15:0]
        db 0x00                               ; Base[23:16]
        db 10011010b                          ; Access: exec/read, present
        db 00100000b                          ; Flags: L=1, D=0
        db 0x00                               ; Base[31:24]

    CODE_SEG equ .code_descriptor - gdt_start ; Offset to the code segment (will be 8 bytes)    
    DATA_SEG equ .data_descriptor - gdt_start ; Offset to the data segment (will be 16 bytes)
    LONG_CODE_SEG equ .long_mode_code_descriptor - gdt_start

gdt_end:

gdt_descriptor:

    ; Calculate GDT start & length to be loaded into lgtd 
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start               ; address (32 bit)
