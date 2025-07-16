ensure_a20:
    call check_a20
    jne .done                 ; Already enabled

    call enable_a20_fast
    call check_a20
    jne .done                 ; Successfully enabled

    jmp halt_with_failure     ; Still not working
.done:
    ret

check_a20:
    ; Set up memory segments
    cli
    mov ax, 0x0000
    mov ds, ax
    mov di, 0x0500 
    not ax                       ; Make ax = 0xFFFF (0xFFFF:0x0510 would map to 0x0000:0x0500 if a20 disabled)
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
    mov bl, [es:si]

    ; Restore originals
    mov [ds:di], al
    mov [es:si], ah

    sti 
    jne switch_to_pm             ; ZF != 0 if different (A20 enabled)
    ret

; -------------------------
enable_a20_fast:
    ; enable a20 via fast method
    in al, 0x92
    or al, 0x02
    out 0x92, al
    ret

halt_with_failure:
    mov si, A20_FAILED
    call print
.hang:
    hlt
    jmp $