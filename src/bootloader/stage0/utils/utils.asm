load_root_dir_to_memo:
    
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
    je .get_root_dir
    inc al                                    ; Length of the root directory

.get_root_dir:
    mov cl, al                                ; Store root dir length in cl
    pop ax                                    ; Retreive starting LBA of a root directory
    mov ah, cl                                ; AH = sectors to read; AL - LBA
    mov bx, buffer                            ; ES:BX is where our stuff will be dumped
    call disk_read                            ; AH = sectors to read; AL - LBA; ES:BX - memo to dump

    xor bx,bx                                 ; Clean bx from address where you dumped root directory
    mov di, buffer                            ; Set di to address of dumped root directory

.done:
    ret



;===============================| Search file in root dir |=============================== 
; Input:
; - SI = address of a file name (e.g., "STAGE1  BIN")
; - DI = addres of the root directory (e.g. , 0x7E00)
; - BX = 0 which will be used as a loop counter (i.e. comparing to bdb_dir_entries_count)
;
; Output:
; - AX = starting file cluster

search_file:
    push di                                   ; Preserve di since cmpsb auto incremetns both (si & di)
    push si
    mov cx, 11                                ; Set comparison counter to 11 bytes (filename (8 bytes) + file format (3 bytes))
    REPE CMPSB                                ; Compare exactly all 11 bytes at si:di
    pop si
    pop di                                    ; Restore original di
    je .foundStage1                            ; ZF = 1 if a match is found. di will contain address of first character in the name

    add di, 32                                ; Go to next record in root folder (+32 bytes) 
    inc bx                                    ; Save the number of records that were searched 
    cmp bx, [bdb_dir_entries_count]           ; If all record search then print that kernel wasn't found
    jl search_file
    jmp .stage1NotFound

.stage1NotFound:
    mov si, msg_file_not_found
    call print
    jmp halt

.foundStage1:
    
    ; Save starting stage1 cluster found from root directory
    mov si, msg_file_found
    call print
    mov ax, [di+26]                ; Get first logical cluster (LBA for the cluster)
    ret