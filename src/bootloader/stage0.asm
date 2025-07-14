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

    ; Calculate LBA of root directory
    mov ax, [bdb_sectors_per_fat]             ; Get number of sectors each FAT table takes
    mov bl, [bdb_fat_count]                   ; Get total sectors all FAT tables take 
    mul bx                                    ;  
    add ax, [bdb_reserved_sectors]            ; Add reserved sectors before FATs
    push ax                                   ; Save LBA of a root directory
 
    ; Calculate length of root directory
    mov ax, [bdb_dir_entries_count] 
    shl ax, 5                                 ; max_files * 32 bits/file = total size of root dir
    div WORD [bdb_bytes_per_sector]           ; Lenght of root dir in sectors
    test dx, dx                               ; Check if remainder = 0
    je rootDirAfter
    inc al                                    ; Length of the root directory

rootDirAfter:
    mov cl, al                                ; Store root dir length in cl
    pop ax                                    ; Retreive starting LBA of a root directory
    mov ah, cl                                ; AH = sectors to read; AL - LBA
    mov bx, buffer                            ; ES:BX is where our stuff will be dumped
    call disk_read

    xor bx,bx                                 ; Clean bx from address where you dumped root directory
    mov di, buffer                            ; Set di to address of dumped root directory

searchStage1:
    mov si, file_stage_1                      ; move stage1 bin file name into si
    mov cx, 11                                ; Set comparison counter to 11 bytes (filename (8 bytes) + file format (3 bytes))
    push di                                   ; Preserve di since cmpsb auto incremetns both (si & di) 
    REPE CMPSB                                ; Compare exactly all 11 bytes at si:di
    pop di                                    ; Restore original di
    je .foundStage1                            ; ZF = 1 if a match is found. di will contain address of first character in the name

    add di, 32                                ; Go to next record in root folder (+32 bytes) 
    inc bx                                    ; Save the number of records that were searched 
    cmp bx, [bdb_dir_entries_count]           ; If all record search then print that kernel wasn't found
    jl searchStage1
    jmp .stage1NotFound

.stage1NotFound:
    mov si, msg_stage1_not_found
    call print
    jmp halt

.foundStage1:
    
    ; Save starting stage1 cluster found from root directory
    mov si, msg_stage1_found
    call print
    mov ax, [di+26]                ; Get first logical cluster (LBA for the cluster)
    mov [stage1_cluster], ax       ; Save starting kernel cluster

    ; Load FAT table into memory
    mov si, msg_moving_fat_to_ram
    call print
    mov bx, buffer
    mov al, [bdb_reserved_sectors] ; Starting LBA of a FAT table
    mov ah, [bdb_sectors_per_fat]  ; Sectors to read (TAT table length)
    call disk_read

    ; Set up memory to load kernel clusters
    mov bx, stage1_load_segment
    mov es, bx
    mov bx, stage1_load_offset ; (0x7cbd in gdb)

%include "./src/bootloader/get_fat_cluster_chain.asm"
%include "./src/bootloader/read_disk.asm"
%include "./src/bootloader/utils.asm"

read_failure:           DB "Failed to read disk!", 0x0D, 0x0A, 0x00
disk_read_sucessfully:  DB "Disk read successful", 0x0D, 0x0A, 0x00
file_stage_1:           DB "STAGE1  BIN", 0x0D, 0x0A, 0x00
msg_stage1_not_found:   DB "STAGE1.BIN not found!", 0x0D, 0x0A, 0x00
msg_stage1_found:       DB "STAGE1.BIN found!", 0x0D, 0x0A, 0x00
msg_moving_fat_to_ram:  DB "Moving FAT12", 0x0D, 0x0A, 0x00
stage1_cluster:         DW 0

stage1_load_segment     EQU 0x9000
stage1_load_offset      EQU 0

TIMES 510-($-$$) DB 0
DW 0xAA55

buffer: