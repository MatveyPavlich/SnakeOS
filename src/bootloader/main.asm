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
    MOV ax, 0x0000      ; Get 0 into ax to later use as memory segment number
    MOV ds, ax          ; Pick first memory segment for data
    MOV es, ax          ; Pick first memory segment for extra segment also
    MOV ss, ax          ; Pick first memory segment for stack segment also

    MOV sp, 0x7C00      ; Make stack occumy memory above bootloader code
    MOV si, os_boot_msg ; Set string pointer to msg start
    CALL print
    HLT

halt:
    JMP halt

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

TIMES 510-($-$$) DB 0
DW 0xAA55