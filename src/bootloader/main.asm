ORG 0x7C00
BITS 16

JMP SHORT main
NOP

bdb_oem:         DB     'MSWIN4.1'
bdb_bytes_per_sector:       DW 512 ; (0x0200 in hex)
bdb_sectors_per_cluster:    DB 1
bdb_reserved_sectors:       DW 1
bdb_fat_count:              DB 2
bdb_dir_entries_count:      DW 0E0h
bdb_total_sectors:          DW 2880
bdb_media_descriptor_type:  DB 0F0h
bdb_sectors_per_fat:        DW 9
bdb_sectors_per_track:      DW 18
bdb_heads:                  DW 2
bdb_hidden_sectors:         DD 0
bdb_large_sector_count:     DD 0

ebr_drive_number:           DB 0
                            DB 0              ; Reserved, often used for "current head"
ebr_signature:              DB 29h
ebr_volume_id:              DB 12h,34h,56h,78h
ebr_volume_label:           DB 'MatthewOS  '  ; Must be exactly 11 bytes
ebr_system_id:              DB 'FAT12   '     ; Must be exactly 8 bytes


; Initialise data, extra and stack segments
main:
    MOV AX, 0x0000
    MOV DS, AX
    MOV ES, AX
    MOV SS, AX

    MOV SP, 0x7C00
    MOV SI, os_boot_msg
    CALL print
    HLT

halt:
    JMP halt

print:
    PUSH SI
    PUSH AX
    PUSH BX

print_loop:
    LODSB
    OR AL,AL
    JZ done_print

    MOV ah, 0x0E
    MOV bh, 0
    int 0x10

    JMP print_loop

done_print:
    POP bx
    POP ax
    POP si
    RET

os_boot_msg: DB "Welcome to MatthewOS.", 0x0D, 0x0A, 0x00

TIMES 510-($-$$) DB 0
DW 0xAA55