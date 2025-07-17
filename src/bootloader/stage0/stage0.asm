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
ebr_volume_label:           DB 'SnakeOS    '  ; Must be exactly 11 bytes
ebr_system_id:              DB 'FAT12   '     ; Magical value that tells it is FAT12 (must be 8 bytes)

main:
    
    ; Set up all segments to the same 64kB chunk
    mov ax, 0                                 
    mov ds, ax                                
    mov es, ax                                
    mov ss, ax                                
    mov sp, 0x7C00                            ; Grow stack below above the code

    call load_root_dir_to_memo                ; Load root dir into mmemo
    mov si, file_kernel                       ; move stage1 bin file name into si
    mov cx, 11                                ; Set comparison counter to 11 bytes (filename (8 bytes) + file format (3 bytes))
    call search_file
    mov [kernel_cluster], ax

    xor bx, bx
    mov di, buffer
    mov si, file_stage_1                      ; move stage1 bin file name into si
    mov cx, 11                                ; Set comparison counter to 11 bytes (filename (8 bytes) + file format (3 bytes))
    call search_file
    mov [stage1_cluster], ax

    ; Load FAT table into memory
    mov si, msg_moving_fat_to_ram
    call print
    mov bx, buffer
    mov al, [bdb_reserved_sectors] ; Starting LBA of a FAT table
    mov ah, [bdb_sectors_per_fat]  ; Sectors to read (TAT table length)

    ; Set up memory to load kernel clusters
    mov bx, stage1_load_segment
    mov es, bx
    mov bx, stage1_load_offset ; (0x7cbd in gdb)

%include "./src/bootloader/stage0/utils/utils.asm"
%include "./src/bootloader/shared/load_cluster_chain.asm"
%include "./src/bootloader/shared/disk_read.asm"
%include "./src/bootloader/shared/utils.asm"

read_failure:           db "Disk read fail", 0x0D, 0x0A, 0x00
disk_read_sucessfully:  db "Disk read done", 0x0D, 0x0A, 0x00
file_stage_1:           db "STAGE1  BIN"
file_kernel:            db "KERNEL  BIN"
msg_file_not_found:     db "File not found", 0x0D, 0x0A, 0x00
msg_file_found:         db "File found", 0x0D, 0x0A, 0x00
msg_moving_fat_to_ram:  db "Get FAT12", 0x0D, 0x0A, 0x00
stage1_cluster:         dw 0
kernel_cluster:         dw 0

stage1_load_segment     equ 0x9000
stage1_load_offset      equ 0

TIMES 510-($-$$) DB 0
dw 0xAA55

; 0x7E00 label
buffer: