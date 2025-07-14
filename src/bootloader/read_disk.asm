; ==============================| Disk stuff |============================== 

; Input: AH = sectors to read; AL = LBA, BX = memo address to dump disk read
; Output: AH = status ; AL = sectors read; CF = 0 on success, 1 on error
disk_read:
    push dx                                   ; Will be using dx for division => preserve old value
    push di                                   ; Will be using di for a counterdisk re-try counter
    push cx                                   ; Will be 
    push ax                                   ; Preserve AH with sectors_to_read, last since will be popped first

    mov ah, 0                                 ; Remove sectors to read from ax register
    call lba_to_chs                           ; Convert LBA to CHS to be able to use BIOS interrupt
    pop ax
    shr ax, 8                                 ; Move sectors_to_read from AH to AL
    mov di, 3                                 ; Ste counter for disk re-tries and drop through to try_again

.try_again:
    mov ah, 0x2                               ; Get disk read interrupt
    stc                                       ; Set carry flag before INT 13h (BIOS uses this)
    int 0x13                                  ; BIOS disk read
    jnc .doneRead                             ; Jump if Carry flag not set
    dec di                                    ; Retry counter -= 1
    test di, di                               ; Is it zero yet?
    jnz .try_again                            ; If not, try_again
    call .diskReset                           ; If read failed, try to reset disk and try_again

.failDiskRead:
    mov si, read_failure
    call print
    jmp halt

.doneRead:
    mov si, disk_read_sucessfully
    call print
    pop cx
    pop di
    pop dx
    ret

.diskReset:
    pusha                                     ; Save all general registers
    mov ah, 0x00                              ; BIOS Reset Disk
    stc
    int 0x13
    jc .failDiskRead                          ; Still failing? Halt
    popa                                      ; Get back all general registers (no need to clean registers beforehand)
    ret

; Input: AX = LBA
; Output: CL = sector (!!!0-5 bits), CH = cylinder (!!!6-15), DH = head, DL = drive number
lba_to_chs:
    xor cx, cx
    xor dx, dx                                ; Clean dx to be used for div remainder
    div WORD [bdb_sectors_per_track]          ; dx = (LBA % sectors per track); ax = LBA / sectors_per_track
    inc dx                                    ; Sector
    mov cx, dx                                ; Save sectors into cx
    xor dx,dx                                 ; Prep dx for division
    div WORD [bdb_heads]                      ; dx = (LBA / sectors_per_track) % heads, ax = (LBA / sectors_per_track) / heads
    mov dh, dl                                ; Head
    mov dl, [ebr_drive_number]                ; Load the drive number
    mov ch, al                                ; Move cylinder to ch
    shl ah, 6                                 ; See int 13,2 page (bottom)
    or cl, ah                                 ; Cylinder
    ret
