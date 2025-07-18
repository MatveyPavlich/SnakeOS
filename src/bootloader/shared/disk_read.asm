; ==============================| disk_read |============================== 
; Input:
;   - AH = sectors to read
;   - AL = LBA
;   - BX = memo address to dump disk read
;   - ES = Segment for disk read dump
; Output (success):
;   - AH = status
;   - AL = sectors read
;   - CF = 0 on success, 1 on error
; Output (fail):
;   - Will halt the whole system
; ==========================================================================

disk_read:
    
    ; Preserve registers that are not params, but are still used within disk_read
    push si                                   ; Will be used in print
    push dx                                   ; Will be used for division
    push di                                   ; Will be used as counter for disk re-try counter
    push cx                                   ; Will be used for lba_to_chs
    
    ; Prepare for disk read BIOS interrupt
    push ax                                   ; Preserve AH with sectors_to_read (last to be popped first)
    mov ah, 0                                 ; Remove sectors to read from ax register
    call lba_to_chs                           ; Convert LBA to CHS to be able to use BIOS interrupt
    pop ax                                    ; Restore sectors to read into AX (will be located in AH)
    shr ax, 8                                 ; Move sectors_to_read from AH to AL
    mov di, 3                                 ; Ste counter for disk re-tries and drop through to try_again

.try_again:
    mov ah, 0x2                               ; Get disk read interrupt
    stc                                       ; Set carry flag before INT 13h (BIOS uses this)
    int 0x13                                  ; BIOS disk read
    jnc .read_successful                      ; Jump if Carry flag not set
    dec di                                    ; Retry counter -= 1
    test di, di                               ; Is it zero yet?
    jnz .try_again                            ; try_again if !=0, else fall through to .read_failed

.read_failed:
    mov si, read_failure
    call print
    jmp halt                                  ; Halt since either stage1 or kernel can't be read (stack not popped)

.read_successful:
    mov si, disk_read_sucessfully
    call print
    pop cx
    pop di
    pop dx
    pop si
    ret

; -----------------------------------------------------------------------------------------
; Input: AX = LBA
; Output: CL = sector (!!!0-5 bits), CH = cylinder (!!!6-15), DH = head, DL = drive number
; -----------------------------------------------------------------------------------------
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
    shl ah, 6                                 ; See int 13,2 bottom of the page
    or cl, ah                                 ; Cylinder
    ret
