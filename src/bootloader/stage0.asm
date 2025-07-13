org 0x7C00                                    ; For assembler to organise the code where first address is 0x7C00
bits 16                                       ; For assembler to know that should be in 16 bits

jmp short main                                ; Jump to code in main (<127 bytes down in memo, so use SHORT)
nop                                           ; Do nothing for 1 CPU cycle

bdb_oem:                    DB 'MSWIN4.1'     ; Tells what formatted the disk
bdb_bytes_per_sector:       DW 512            ; 
bdb_sectors_per_cluster:    DB 1              ; FAT table has offsets to clusters, not sectors! This is why it is important
bdb_reserved_sectors:       DW 1              ; Number of sectors before FAT sector including boot sector => 1 => no additional sectors
bdb_fat_count:              DB 2              ; Number of fat tables (one will be for a backup)
bdb_dir_entries_count:      DW 0xE0           ; Max number of enteries in a root directory
bdb_total_sectors:          DW 2880
bdb_media_descriptor_type:  DB 0xF0           ; Tells 
bdb_sectors_per_fat:        DW 9              ; Each FAT table will take 9 sectors
bdb_sectors_per_track:      DW 18             ; Each track will have 18 sectors (ignore the fact that outer tracks are longer)
bdb_heads:                  DW 2              ; Since we are on the floppy disk we will have 2 heads only
bdb_hidden_sectors:         DD 0                
bdb_large_sector_count:     DD 0

ebr_drive_number:           DB 0x00           ; Hardcoding that we are using a floppy disk
                            DB 0              ; Reserved, often used for "current head"
ebr_signature:              DB 0x29
ebr_volume_id:              DB 12h,34h,56h,78h
ebr_volume_label:           DB 'MatthewOS  '  ; Must be exactly 11 bytes
ebr_system_id:              DB 'FAT12   '     ; Magical value that tells it is FAT12 (must be 8 bytes)

LBA:                        DB 1              ; LBA of stage1
sectors_in_cylinder :       DB 0              ; This is to store result of heads * (sectors / tracks)
disk_stuff_dump_memo:       DW 0x7E00         ; This is where we will dump a first sector from disk
cylinder            :       DB 0
head                :       DB 0
sector              :       DB 0
sectors_to_read     :       DB 2              ; Get 2 sectors just in case

main:

    ; Set up memory segments
    mov ax, 0                                 ; Get 0 into ax to later use as memory segment number
    mov ds, ax                                ; Pick first memory segment for data
    mov es, ax                                ; Pick first memory segment for extra segment also
    mov ss, ax                                ; Pick first memory segment for stack segment also
    mov sp, 0x7C00                            ; Make stack occupy memory above bootloader code
    
    ; Save device that had bootloader (0x00 floppy, 0x80 HHD) BIOS sets dl automatically
    mov [ebr_drive_number], dl
    xor dx, dx                                ; Clean dl
    
    ; Get stage1 into memo
    call disk_read                            
    jmp halt

lba_to_chs:
    mov ax, [bdb_heads]                       ; Get number of heads
    mul WORD [bdb_sectors_per_track]          ; Get total sectors in a cylinder
    mov BYTE [sectors_in_cylinder], al        ; Save value
    xor ax, ax
    mov al, [LBA]                             ; Get LBA
    div BYTE [sectors_in_cylinder]            ; Get cylinder
    mov BYTE [cylinder], al       
    xor al, al
    mov al, ah                                ; Get remainder as sector number
    inc al                                    ; Get the sector number in a head
    xor ah, ah
    div WORD [bdb_sectors_per_track]          ; Find the number of tracks
    mov [head], al
    mov [sector], dl                          ; Save sectors and fall through to disk_read
    ret

disk_read:
    call lba_to_chs                           ; Convert LBA to CHS to be able to use BIOS interrupt
    xor ax, ax
    xor bx, bx
    xor cx, cx
    xor dx, dx

    mov al, [sectors_to_read]                 ; Move number of sectors to read to al
    mov bx, [disk_stuff_dump_memo]            ; ES:BX is where our stuff will be dumped
    mov ch, [cylinder]
    mov cl, [sector]
    mov dh, [head]
    mov dl, [ebr_drive_number]

    mov di, 3                                 ; Ste counter for disk re-tries

retry:
    mov ah, 2                                 ; Get disk read interrupt
    stc                                       ; Set carry flag before INT 13h (BIOS uses this)
    int 0x13                                  ; BIOS disk read
    jnc .doneRead                             ; Jump if Carry flag not set
    dec di                                    ; Retry counter -= 1
    test di, di                               ; Is it zero yet?
    jnz retry                                 ; If not, retry
    call .diskReset                           ; If read failed, try to reset disk and retry
.failDiskRead:
    mov si, read_failure
    call print
    jmp halt
.doneRead:
    mov si, disk_read_sucessfully
    call print
    ret
.diskReset:
    pusha                                     ; Save all general registers
    mov ah, 0x00                              ; BIOS Reset Disk
    stc
    int 0x13
    jc .failDiskRead                          ; Still failing? Halt
    popa                                      ; Get back all general registers (no need to clean registers beforehand)
    ret

; Print stuff
print:
    push si                                   ; Preserve 
    push ax                                   ; Preserve
    push bx                                   ; Preserve bx and fall trhough to code below

print_loop:
    lodsb                                     ; Load DS:SI byte to al, then increment SI
    or al, al                                 ; Hacky way to avoid CMP al, 0x00
    jz done_print                             ; Finish printing if zero
    mov ah, 0x0E                              ; Set ah to 0x0E to access BIOS teletype print
    mov bh, 0                                 ; Set page number to 0
    int 0x10                                  ; Call BIOS interrup
    jmp print_loop

done_print:
    pop bx                                    ; Get bx value from before print loop
    pop ax                                    ; Get ax value from before print loop
    pop si                                    ; Get si value from before print loop
    ret

; Halt stuff
halt:
    hlt
    jmp halt

read_failure:           DB "Failed to read disk!", 0x0D, 0x0A, 0x00
disk_read_sucessfully:  DB "Disk read successful", 0x0D, 0x0A, 0x00
file_stage_1:           DB "STAGE1  BIN", 0x0D, 0x0A, 0x00
msg_stage1_not_found:   DB "STAGE1.BIN not found!", 0x0D, 0x0A, 0x00
msg_stage1_found:       DB "STAGE1.BIN found!", 0x0D, 0x0A, 0x00
stage1_cluster:         DW 0

TIMES 510-($-$$) DB 0
DW 0xAA55

buffer: