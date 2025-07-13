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

LBA:                        DB 1              ; Place to store LBA
sectors_in_cylinder :       DB 0              ; This is to store result of heads * (sectors / tracks)
disk_stuff_dump_memo:       DW 0x7E00         ; This is where we will dump a first sector from disk
cylinder            :       DB 0
head                :       DB 0
sector              :       DB 0
sectors_to_read     :       DB 1

main:
    
    ; Set up all segments
    mov ax, 0                                 
    mov ds, ax                                ; Pick first memory segment for data
    mov es, ax                                ; Pick first memory segment for extra segment also
    mov ss, ax                                ; Pick first memory segment for stack segment also
    mov sp, 0x7C00                            ; Make stack occupy memory above bootloader code
    mov [ebr_drive_number], dl                ; Save device that had bootloader (0x00 floppy, 0x80 HHD) BIOS sets dl automatically
    xor dx, dx                                ; Clean dl

    ; Find LBA of root directory
    mov ax, [bdb_sectors_per_fat]             ; Get number of sectors each FAT table takes
    mov bl, [bdb_fat_count]                   ; Get total sectors all FAT tables take 
    mul bx                                    ;  
    add ax, [bdb_reserved_sectors]            ; Add reserved sectors before FATs
    push ax                                   ; LBA of a root directory
 
    ; Find length of root directory
    mov ax, [bdb_dir_entries_count] 
    shl ax, 5                                 ; max_num_of_files * 32 bits/file = total byte size of root dir
    div WORD [bdb_bytes_per_sector]           ; Lenght of root dir in sectors
    test dx, dx                               ; Check if remainder = 0
    je rootDirAfter
    inc al                                    ; Length of the root directory

rootDirAfter:
    mov cl, al
    mov al, 0
    mov [LBA], al
    mov [sectors_to_read], al
    mov [sectors_to_read], cl
    pop ax
    mov [LBA], al
    mov dl, [ebr_drive_number]
    mov bx, buffer
    call disk_read

    xor bx,bx
    mov di, buffer

searchStage1:
    mov si, file_stage_1                      ; move kernel bin file name into si
    mov cx, 11                                ; Set comparison counter to 11 bytes (filename (8 bytes) + file format (3 bytes))
    push di                                   ; Preserve di since cmpsb auto incremetns both (si & di) 
    REPE CMPSB                                ; Compare exactly all 11 bytes at si:di
    pop di                                    ; Restore original di
    je foundStage1                            ; ZF = 1 if a match is found. di will contain address of first character in the name

    add di, 32                                ; Go to next record in root folder (+32 bytes) 
    inc bx                                    ; Save the number of records that were searched 
    cmp bx, [bdb_dir_entries_count]           ; If all record search then print that kernel wasn't found
    jl searchStage1
    jmp stage1NotFound

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

stage1NotFound:
    mov si, msg_stage1_not_found
    call print
    jmp halt

foundStage1:
    
    ; Save starting kernel cluster found from root directory
    mov si, msg_stage1_found
    call print
    mov ax, [di+26]                ; Get first logical cluster (LBA for the cluster)
    mov [stage1_cluster], ax       ; Save starting kernel cluster

    ; Load FAT table into memory
    mov si, msg_moving_fat_to_ram
    call print
    mov ax, [bdb_reserved_sectors] ; Starting sector of a FAT table
    mov [LBA], al                  ; Load into memo
    mov cl, [bdb_sectors_per_fat]  ; Sectors to read
    mov [sectors_to_read], cl      ; Save sectors to read
    call lba_to_chs
    call disk_read

    ; Set up memory to load kernel clusters
    mov bx, stage1_load_segment
    mov es, bx
    mov bx, stage1_load_offset

; loadKernelLoop:
    
;     ; Load starting kernel cluster into RAM
;     mov ax, [stage1_cluster]
;     mov [LBA], al                  ; Load into memo
;     add ax, 31                     ; Cluster number -> LBA conversion
;     mov cl, 1                      ; Number of sectors we'll read
;     mov [sectors_to_read], cl      ; Read one sector (since cluster is 1 sector)
;     mov dl, [ebr_drive_number]
;     call lba_to_chs
;     call disk_read
;     add bx, [bdb_bytes_per_sector] ; Increment kernel offset to load the next cluster

;     ; Find next pointer in FAT
;     mov ax, [stage1_cluster] ; (kernel cluster * 3)/2
;     mov cx, 3
;     mul cx
;     mov cx, 2
;     div cx

;     mov si, buffer
;     add si, ax
;     mov ax, [ds:si]

;     or dx,dx
;     jz even

; odd:
;     shr ax,4
;     jmp nextClusterAfter
; even:
;     and ax, 0x0FFF

; nextClusterAfter:
;     cmp ax, 0x0FF8
;     jae readFinish

;     mov [stage1_cluster], ax
;     jmp loadKernelLoop

; readFinish:
;     mov dl, [ebr_drive_number]
;     mov ax, stage1_load_segment
;     mov ds,ax
;     mov es,ax
;     jmp stage1_load_segment:stage1_load_offset

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

halt:
    hlt
    jmp halt
    

read_failure:           DB "Failed to read disk!", 0x0D, 0x0A, 0x00
disk_read_sucessfully:  DB "Disk read successful", 0x0D, 0x0A, 0x00
file_stage_1:           DB "STAGE1  BIN", 0x0D, 0x0A, 0x00
msg_stage1_not_found:   DB "STAGE1.BIN not found!", 0x0D, 0x0A, 0x00
msg_stage1_found:       DB "STAGE1.BIN found!", 0x0D, 0x0A, 0x00
msg_moving_fat_to_ram:  DB "Moving FAT12", 0x0D, 0x0A, 0x00
stage1_cluster:         DW 0

stage1_load_segment     EQU 0x7E00
stage1_load_offset      EQU 0

TIMES 510-($-$$) DB 0
DW 0xAA55

buffer: