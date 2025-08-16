print:
    pusha

; while (string[i] != 0) { print string[i]; i++ }
start:
    mov al, [si] ; 'bx' is the base address for the string
    cmp al, 0 
    je done
    mov ah, 0x0e ; the part where we print with the BIOS help
    int 0x10     ; 'al' already contains the char
    add si, 1    ; increment pointer and do next loop
    jmp start

done:
    popa
    ret

halt:
    hlt
    jmp halt


disk_read:
    ; Not the proper way to set up track and cylinder,
    ; but works with the values (sector should be only 6 bits)
    mov ch, 0                                 ; CH = track/cylinder number
    mov dh, 0                                 ; DH = head number
    mov cl, 2                                 ; CL = sector number (1 since 0 is bootloader)
    mov dl, 0                                 ; DL = drive number
    mov bx, 0x9000                            
    mov es, bx                                ; Load at ES:BX => 0x90000
    xor bx, bx 
    mov bx, 0
    mov di, 3                                 ; Try 3 times to read the disk

    .try_again:
        mov ah, 0x2                           ; AH = 02 => access disk read interrupt
        mov al, 0xF                           ; AL = n. of sectors to read
        stc
        int 0x13
        jnc .read_successful
        dec di
        test di, di
        jnz .try_again
    
    .read_failed:
        mov es, bx
        mov si, DISK_READ_FAILED
        call print
        jmp halt
    
    .read_successful:
        mov es, bx
        mov si, DISK_READ_SUCCESSFULLY
        call print
        ret

DISK_READ_SUCCESSFULLY: db "Read success", 0x00
DISK_READ_FAILED: db "Read failed. Halt", 0x00 