ORG 0x7C00                                     ; For assembler to organise the code where first address is 0x7C00
BITS 16                                        ; For assembler to know that should be in 16 bits

JMP SHORT main                                 ; Jump to code in main (<127 bytes down in memo, so use SHORT)
NOP                                            ; Do nothing for 1 CPU cycle

bdb_oem:                    DB 'MSWIN4.1'      ; Tells what formatted the disk
bdb_bytes_per_sector:       DW 512             ; 
bdb_sectors_per_cluster:    DB 1               ; FAT table has offsets to clusters, not sectors! This is why it is important
bdb_reserved_sectors:       DW 1               ; Number of sectors before FAT sector including boot sector => 1 => no additional sectors
bdb_fat_count:              DB 2               ; Number of fat tables (one will be for a backup)
bdb_dir_entries_count:      DW 0xE0            ; Max number of enteries in a root directory
bdb_total_sectors:          DW 2880
bdb_media_descriptor_type:  DB 0xF0            ; Tells 
bdb_sectors_per_fat:        DW 9               ; Each FAT table will take 9 sectors
bdb_sectors_per_track:      DW 18              ; Each track will have 18 sectors (ignore the fact that outer tracks are longer)
bdb_heads:                  DW 2               ; Since we are on the floppy disk we will have 2 heads only
bdb_hidden_sectors:         DD 0                
bdb_large_sector_count:     DD 0

ebr_drive_number:           DB 0x00           ; Hardcoding that we are using a floppy disk
                            DB 0              ; Reserved, often used for "current head"
ebr_signature:              DB 0x29
ebr_volume_id:              DB 12h,34h,56h,78h
ebr_volume_label:           DB 'MatthewOS  '  ; Must be exactly 11 bytes
ebr_system_id:              DB 'FAT12   '     ; Must be exactly 8 bytes


main:
    MOV ax, 0                                 ; Get 0 into ax to later use as memory segment number
    MOV ds, ax                                ; Pick first memory segment for data
    MOV es, ax                                ; Pick first memory segment for extra segment also
    MOV ss, ax                                ; Pick first memory segment for stack segment also

    MOV sp, 0x7C00                            ; Make stack occupy memory above bootloader code

    ; Set up disk stuff (WIP)
    MOV [ebr_drive_number], dl                ; Save device that had bootloader (0x00 -> floppy, 0x80 -> HHD) BIOS sets dl automatically
    MOV ax, 1                                 ; LBA block to use in a disk_read (0 is bootloader)
    MOV cl, 1                                 ; Sector cylinder number on the disk
    MOV bx, 0x07E00                           ; Pointer to memory buffer to write disk sectors. Will be written to ES:BX
    CALL disk_read              

    MOV si, os_boot_msg                       ; Set string pointer to msg start
    CALL print
    HLT

halt:
    JMP halt

; --------------------------(WIP)--------------------
; Input:  ax = LBA
; Output: ch = cylinder | cl = sector | dh = head 
lba_to_chs:
    PUSH ax                          
    PUSH dx

    XOR dx,dx
    DIV word [bdb_sectors_per_track] ;(LBA % sectors per track) + 1 <- sector
    INC dx  ;Sector
    MOV cx,dx

    XOR dx,dx
    DIV word [bdb_heads]

    MOV dh,dl ;head
    MOV ch,al
    SHL ah, 6
    OR CL, AH   ;cylinder

    ; POP ax
    ; MOV dl,al
    POP dx
    POP ax

    RET

disk_read:
    PUSH ax                    ; Preserve number of sectors to read
    PUSH bx                    ; Preserver pointer to the memory buffer to dump sectors from disk      
    PUSH cx                    ; Preserve sector number
    PUSH dx                    ; Preserver device identifier

    call lba_to_chs
    ; Load drive number and sector count
    mov dl, [ebr_drive_number]
    mov al, 1

    MOV ah, 0x2
    MOV di, 3   ;counter

retry:
    STC          ; Way to set up a carry in BIOS
    INT 13h
    jnc doneRead
 
    call diskReset

    DEC di
    TEST di,di
    JNZ retry

failDiskRead:
    MOV si, read_failure
    CALL print
    HLT
    JMP halt

diskReset:
    pusha
    MOV ah,0
    STC
    INT 13h
    JC failDiskRead
    POPA
    RET

doneRead:
    pop di
    pop dx
    pop cx
    pop bx
    pop ax

    ret

print:
    PUSH si             ; Preserve 
    PUSH ax
    PUSH bx             ; Preserve bx and fall trhough to code below

print_loop:
    LODSB               ; Load DS:SI byte to al, then increment SI
    OR al, al           ; Hacky way to avoid CMP al, 0x00
    JZ done_print       ; Finish printing if zero

    MOV ah, 0x0E        ; Set ah to 0x0E to access BIOS teletype print
    MOV bh, 0           ; Set page number to 0
    int 0x10            ; Call BIOS interrup

    JMP print_loop

done_print:
    POP bx             ; Get bx value from before print loop
    POP ax             ; Get ax value from before print loop
    POP si             ; Get si value from before print loop
    RET

os_boot_msg: DB "Welcome to MatthewOS.", 0x0D, 0x0A, 0x00
read_failure DB "Failed to read disk!", 0x0D, 0x0A, 0

TIMES 510-($-$$) DB 0
DW 0xAA55

buffer: