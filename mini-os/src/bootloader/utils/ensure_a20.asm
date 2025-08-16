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

A20_FAILED:    db "A20", 0xD, 0xA, 0x00

