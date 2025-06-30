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


; Initialise data, extra and stack segments
main:
    MOV ax, 0x0000      ; Get 0 into ax to later use as data, code and extra segment
    MOV ds, ax          ; Use first data segment
    MOV es, ax          ; Use first extra segment
    MOV ss, ax          ; Use first stack segment

    MOV sp, 0x7C00
    MOV si, os_boot_msg ; Set string pointer to msg start
    CALL print
    HLT

halt:
    JMP halt

print:
    PUSH si
    PUSH ax
    PUSH bx

print_loop:
    LODSB
    OR al, al
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