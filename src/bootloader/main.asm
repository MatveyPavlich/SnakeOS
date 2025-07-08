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
ebr_system_id:              DB 'FAT12   '     ; Magical value that tells it is FAT12 (must be 8 bytes)

LBA:                        DB 1              ; Place to store LBA
sectors_in_cylinder :       DB 0              ; This is to store result of heads * (sectors / tracks)
disk_stuff_dump_memo:       DW 0x7E00         ; This is where we will dump a first sector from disk
cylinder            :       DB 0
head                :       DB 0
sector              :       DB 0
sectors_to_read     :       DB 1

main:
    MOV ax, 0                                 ; Get 0 into ax to later use as memory segment number
    MOV ds, ax                                ; Pick first memory segment for data
    MOV es, ax                                ; Pick first memory segment for extra segment also
    MOV ss, ax                                ; Pick first memory segment for stack segment also

    MOV sp, 0x7C00                            ; Make stack occupy memory above bootloader code

    ; Set up disk stuff (WIP)
    MOV [ebr_drive_number], dl                ; Save device that had bootloader (0x00 floppy, 0x80 HHD) BIOS sets dl automatically
    XOR dl, dl                                ; Clean dl
    CALL lba_to_chs
    CALL disk_read             

    MOV si, os_boot_msg                       ; Set string pointer to msg start
    CALL print
    HLT

; AH = 02
; AL = number of sectors to read	(1-128 dec.)
; CH = track/cylinder number  (0-1023 dec., see below)
; CL = sector number  (1-17 dec.)
; DH = head number  (0-15 dec.)
; DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
; ES:BX = pointer to buffer

halt:
    JMP halt

; Input:  ax = LBA
; Output: ch = cylinder | cl = sector | dh = head 
lba_to_chs:
    MOV ax, [bdb_heads]                ; Get number of heads
    MUL WORD [bdb_sectors_per_track]   ; Get total sectors in a cylinder
    MOV BYTE [sectors_in_cylinder], al ; Save value
    XOR ax, ax
    MOV al, [LBA]                      ; Get LBA
    DIV BYTE [sectors_in_cylinder]     ; Get cylinder
    MOV BYTE [cylinder], al
    XOR al, al
    MOV al, ah                         ; Get remainder as sector number
    INC al                             ; Get the sector number in a head
    XOR ah, ah
    DIV WORD [bdb_sectors_per_track]   ; Find the number of tracks
    MOV [head], al
    MOV [sector], ah                   ; Save sectors and fall through to disk_read

disk_read:
    XOR ax, ax
    XOR bx, bx
    XOR cx, cx
    XOR dx, dx

    MOV al, [sectors_to_read]           ; Move number of sectors to read to al
    MOV bx, [disk_stuff_dump_memo]      ; ES:BX is where our stuff will be dumped
    MOV ch, [cylinder]
    MOV cl, [sector]
    MOV dh, [head]
    MOV dl, [ebr_drive_number]

    MOV di, 3                            ; counter

.retry:
    MOV ah, 2                           ; Get disk read interrupt
    STC                                 ; Set carry flag before INT 13h (BIOS uses this)
    INT 13h                             ; BIOS disk read
    JNC .doneRead                       ; Jump if Carry flag not set
    
    DEC di                              ; Retry counter -= 1
    TEST di, di                         ; Is it zero yet?
    JNZ .retry                          ; If not, retry
    CALL diskReset                      ; If read failed, try to reset disk and retry

.failDiskRead:
    MOV si, read_failure
    CALL print
    HLT

diskReset:
    pusha                               ; Save all general registers
    MOV ah, 0x00                        ; BIOS Reset Disk
    STC
    INT 13h
    jc .failDiskRead                    ; Still failing? Halt
    popa                                ; Get back all general registers (no need to clean registers beforehand)
    RET

.doneRead:
    MOV si, disk_read_sucessfully
    CALL print
    RET

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

os_boot_msg          : DB "Welcome to MatthewOS.", 0x0D, 0x0A, 0x00
read_failure         : DB "Failed to read disk!", 0x0D, 0x0A, 0x00
disk_read_sucessfully: DB "Disk read, all good", 0x0D, 0x0A, 0x00

TIMES 510-($-$$) DB 0
DW 0xAA55

buffer: