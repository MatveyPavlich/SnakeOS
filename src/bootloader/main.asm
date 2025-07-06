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

main:
    MOV ax, 0                                 ; Get 0 into ax to later use as memory segment number
    MOV ds, ax                                ; Pick first memory segment for data
    MOV es, ax                                ; Pick first memory segment for extra segment also
    MOV ss, ax                                ; Pick first memory segment for stack segment also

    MOV sp, 0x7C00                            ; Make stack occupy memory above bootloader code

    ; Set up disk stuff (WIP)
    MOV [ebr_drive_number], dl                ; Save device that had bootloader (0x00 floppy, 0x80 HHD) BIOS sets dl automatically
    XOR dl, dl                                ; Clean dl
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

; --------------------------(WIP)--------------------
; Input:  ax = LBA
; Output: ch = cylinder | cl = sector | dh = head 
lba_to_chs:
    ; MOV al, [LBA]                    ; Get LBA
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
    DIV WORD [bdb_sectors_per_track]   ; do sectors / 
    MOV [head], al
    MOV [sector], ah
    XOR bx, bx
    MOV bl, [cylinder] 

    ; DIV word [bdb_sectors_per_track] ;
    ; INC dx  ;Sector
    ; MOV cx,dx

    ; XOR dx,dx
    ; DIV word [bdb_heads]

    ; MOV dh,dl ;head
    ; MOV ch,al
    ; SHL ah, 6
    ; OR CL, AH   ;cylinder

    ; ; POP ax
    ; ; MOV dl,al
    ; POP dx
    ; POP ax

    RET

disk_read:
    call lba_to_chs
;     mov dl, [ebr_drive_number]
;     mov al, 1

;     MOV ah, 0x2
;     MOV di, 3   ;counter

; retry:
;     STC          ; Way to set up a carry in BIOS
;     INT 13h
;     jnc doneRead
 
;     call diskReset

;     DEC di
;     TEST di,di
;     JNZ retry

; failDiskRead:
;     MOV si, read_failure
;     CALL print
;     HLT
;     JMP halt

; diskReset:
;     pusha
;     MOV ah,0
;     STC
;     INT 13h
;     JC failDiskRead
;     POPA
;     RET

; doneRead:
;     pop di
;     pop dx
;     pop cx
;     pop bx
;     pop ax

;     ret

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