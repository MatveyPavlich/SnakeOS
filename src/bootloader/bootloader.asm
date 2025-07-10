ORG 0x7C00                                    ; For assembler to organise the code where first address is 0x7C00
BITS 16                                       ; For assembler to know that should be in 16 bits

JMP SHORT main                                ; Jump to code in main (<127 bytes down in memo, so use SHORT)
NOP                                           ; Do nothing for 1 CPU cycle

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
    MOV [ebr_drive_number], dl                ; Save device that had bootloader (0x00 floppy, 0x80 HHD) BIOS sets dl automatically
    XOR dx, dx                                ; Clean dl
    CALL lba_to_chs                           ; Convert LBA to CHS to be able to use BIOS interrupt
    CALL disk_read                            ; Read disk using CHS values
    CALL compute_root_dir_stuff               ; WIP: computing root start, length and offset to the first data segment
    HLT

lba_to_chs:
    MOV ax, [bdb_heads]                       ; Get number of heads
    MUL WORD [bdb_sectors_per_track]          ; Get total sectors in a cylinder
    MOV BYTE [sectors_in_cylinder], al        ; Save value
    XOR ax, ax
    MOV al, [LBA]                             ; Get LBA
    DIV BYTE [sectors_in_cylinder]            ; Get cylinder
    MOV BYTE [cylinder], al       
    XOR al, al
    MOV al, ah                                ; Get remainder as sector number
    INC al                                    ; Get the sector number in a head
    XOR ah, ah
    DIV WORD [bdb_sectors_per_track]          ; Find the number of tracks
    MOV [head], al
    MOV [sector], dl                          ; Save sectors and fall through to disk_read
    RET

disk_read:
    XOR ax, ax
    XOR bx, bx
    XOR cx, cx
    XOR dx, dx

    MOV al, [sectors_to_read]                 ; Move number of sectors to read to al
    MOV bx, [disk_stuff_dump_memo]            ; ES:BX is where our stuff will be dumped
    MOV ch, [cylinder]
    MOV cl, [sector]
    MOV dh, [head]
    MOV dl, [ebr_drive_number]

    MOV di, 3                                 ; Ste counter for disk re-tries

retry:
    MOV ah, 2                                 ; Get disk read interrupt
    STC                                       ; Set carry flag before INT 13h (BIOS uses this)
    INT 13h                                   ; BIOS disk read
    JNC .doneRead                             ; Jump if Carry flag not set
    DEC di                                    ; Retry counter -= 1
    TEST di, di                               ; Is it zero yet?
    JNZ retry                                 ; If not, retry
    CALL .diskReset                           ; If read failed, try to reset disk and retry

.failDiskRead:
    MOV si, read_failure
    CALL print
    HLT

.doneRead:
    MOV si, disk_read_sucessfully
    CALL print
    RET

.diskReset:
    pusha                                     ; Save all general registers
    MOV ah, 0x00                              ; BIOS Reset Disk
    STC
    INT 13h
    jc .failDiskRead                          ; Still failing? Halt
    popa                                      ; Get back all general registers (no need to clean registers beforehand)
    RET

print:
    PUSH si                                   ; Preserve 
    PUSH ax                                   ; Preserve
    PUSH bx                                   ; Preserve bx and fall trhough to code below

print_loop:
    LODSB                                     ; Load DS:SI byte to al, then increment SI
    OR al, al                                 ; Hacky way to avoid CMP al, 0x00
    JZ done_print                             ; Finish printing if zero
    MOV ah, 0x0E                              ; Set ah to 0x0E to access BIOS teletype print
    MOV bh, 0                                 ; Set page number to 0
    int 0x10                                  ; Call BIOS interrup

    JMP print_loop

done_print:
    POP bx                                    ; Get bx value from before print loop
    POP ax                                    ; Get ax value from before print loop
    POP si                                    ; Get si value from before print loop
    RET

compute_root_dir_stuff:
    xor al, al
    xor bh, bh

    mov ax, [bdb_sectors_per_fat]             ; Get number of sectors each FAT table takes
    mov bl, [bdb_fat_count]                   ; Get total sectors all FAT tables take 
    mul bx                                    ;  
    add ax, [bdb_reserved_sectors]            ; Add reserved sectors before FATs => LBA of a root dir
    push ax

    xor bx, bx
    xor ax, ax
    xor dx, dx                                ; Remainder after multiplying is in dx 
    mov ax, [bdb_dir_entries_count] 
    shl ax, 5                                 ; max_num_of_files * 32 bits/file = total byte size of root dir
    div WORD [bdb_bytes_per_sector]           ; Lenght of root dir in sectors
    test dx, dx                               ; Check if remainder = 0
    je rootDirAfter
    inc al

; WIP, make sure to rewrite LBA and sectors_to_read from memo
rootDirAfter:
    mov cl, al
    mov al, 0
    mov [LBA], al
    mov BYTE [sectors_to_read], 0
    pop ax
    mov [LBA], al
    mov dl, [ebr_drive_number]
    MOV bx, buffer
    CALL disk_read

    XOR bx,bx
    MOV di,buffer

searchKernel:
    MOV si, file_kernel_bin                   ; move kernel bin file name into si
    MOV cx, 11                                ; Set comparison counter to 11 bytes (filename (8 bytes) + file format (3 bytes))
    PUSH di                                   ; Preserve di since cmpsb auto incremetns both (si & di) 
    REPE CMPSB                                ; Compare exactly all 11 bytes at si:di
    POP di                                    ; Restore original di
    JE foundKernel                            ; ZF = 1 if a match is found

    ADD di, 32                                ; Go to next record in root folder (+32 bytes) 
    INC bx                                    ; Save the number of records that were searched 
    CMP bx, [bdb_dir_entries_count]           ; If all record search then print that kernel wasn't found
    JL searchKernel

    JMP kernelNotFound

kernelNotFound:
    MOV si, msg_kernel_not_found
    CALL print

    HLT
    JMP halt

foundKernel:
    mov si, msg_kernel_found
    CALL print
;     MOV ax, [di+26]
;     MOV [kernel_cluster], ax

;     MOV ax, [bdb_reserved_sectors]
;     MOV bx, buffer 
;     MOV cl, [bdb_sectors_per_fat]
;     MOV dl, [ebr_drive_number]

;     CALL disk_read

;     MOV bx, kernel_load_segment
;     MOV es,bx
;     MOV bx, kernel_load_offset

; loadKernelLoop:
;     MOV ax, [kernel_cluster]
;     ADD ax, 31
;     MOV cl, 1
;     MOV dl, [ebr_drive_number]

;     CALL disk_read

;     ADD bx, [bdb_bytes_per_sector]

;     MOV ax, [kernel_cluster] ;(kernel cluster * 3)/2
;     MOV cx, 3
;     MUL cx
;     MOV cx, 2
;     DIV cx

;     MOV si, buffer
;     ADD si, ax
;     MOV ax, [ds:si]

;     OR dx,dx
;     JZ even

; odd:
;     SHR ax,4
;     JMP nextClusterAfter
; even:
;     AND ax, 0x0FFF

; nextClusterAfter:
;     CMP ax, 0x0FF8
;     JAE readFinish

;     MOV [kernel_cluster], ax
;     JMP loadKernelLoop

; readFinish:
;     MOV dl, [ebr_drive_number]
;     MOV ax, kernel_load_segment
;     MOV ds,ax
;     MOV es,ax

;     JMP kernel_load_segment:kernel_load_offset

;     HLT

halt:
    JMP halt
    

read_failure:           DB "Failed to read disk!", 0x0D, 0x0A, 0x00
disk_read_sucessfully:  DB "Disk read successful", 0x0D, 0x0A, 0x00
file_kernel_bin:        DB "KERNEL  BIN", 0x0D, 0x0A, 0x00
msg_kernel_not_found:   DB "KERNEL.BIN not found!", 0x0D, 0x0A, 0x00
msg_kernel_found:       DB "KERNEL.BIN found!", 0x0D, 0x0A, 0x00
kernel_cluster;         DW 0

kernel_load_segment EQU 0x2000
kernel_load_offset EQU 0

TIMES 510-($-$$) DB 0
DW 0xAA55

buffer: