ORG 0x7C00
BITS 16

JMP SHORT main
NOP

bdb_oem:         DB     'MSWIN4.1'
bdb_bytes_per_sector:       DW 512 ; (0x0200 in hex)
bdb_sectors_per_cluster:    DB 1
bdb_reserved_sectors:       DW 1
bdb_fat_count:              DB 2
bdb_dir_entries_count:      DW 0xE0
bdb_total_sectors:          DW 2880
bdb_media_descriptor_type:  DB 0xF0
bdb_sectors_per_fat:        DW 9
bdb_sectors_per_track:      DW 18
bdb_heads:                  DW 2
bdb_hidden_sectors:         DD 0
bdb_large_sector_count:     DD 0

ebr_drive_number:           DB 0
                            DB 0              ; Reserved, often used for "current head"
ebr_signature:              DB 0x29
ebr_volume_id:              DB 0x12, 0x34, 0x56, 0x78
ebr_volume_label:           DB 'MatthewOS  '  ; Must be exactly 11 bytes
ebr_system_id:              DB 'FAT12   '     ; Must be exactly 8 bytes


main:
    MOV ax, 0x0000              ; Get 0 into ax to later use as memory segment number
    MOV ds, ax                  ; Pick first memory segment for data
    MOV es, ax                  ; Pick first memory segment for extra segment also
    MOV ss, ax                  ; Pick first memory segment for stack segment also

    ; Set up disk stuff (WIP)
    MOV [ebr_drive_number], dl  ; I think the guy is wrong here
    MOV ax, 1                   ; LBA block to use in a disk_read
    MOV cl, 1                   ; Sector number on the disk
    MOV bx, 0x07E00 ; ???
    CALL disk_read

    MOV sp, 0x7C00              ; Make stack occumy memory above bootloader code
    MOV si, os_boot_msg         ; Set string pointer to msg start
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

    POP ax
    MOV dl,al
    POP ax

    RET

disk_read:
    PUSH ax
    PUSH bx
    PUSH cx
    PUSH dx
    PUSH di

    call lba_to_chs

    MOV ah, 02h
    MOV di, 3   ;counter

retry:
    STC
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
file_kernel_bin DB "KERNEL  BIN"
msg_kernel_not_found DB "KERNEL.BIN not found!"
kernel_cluster DW 0

kernel_load_segment EQU 0x2000
kernel_load_offset EQU 0

TIMES 510-($-$$) DB 0
DW 0xAA55

buffer: