;===============================| Load root directory |===============================
; Input: void
; Output:
;   - Root directory will be loaded to 0x7E00 (i.e., address of the buffer label)
;   - Clobbers: AX, BX, CX (might not be an exhaustive list)
;======================================================================================

load_root_dir:
    
    ; Calculate LBA of root directory
    mov ax, [bdb_sectors_per_fat]             ; Get number of sectors each FAT table takes
    mov bl, [bdb_fat_count]                   ; Get total sectors all FAT tables take 
    mul bx                                    ;  
    add ax, [bdb_reserved_sectors]            ; Add reserved sectors before FATs
    push ax                                   ; Preserve LBA of a root directory
 
    ; Calculate length of root directory
    mov ax, [bdb_dir_entries_count] 
    shl ax, 5                                 ; max_files * 32 bits/file = total size of root dir
    div WORD [bdb_bytes_per_sector]           ; Lenght of root dir in sectors
    test dx, dx                               ; Check if remainder = 0
    je .get_root_dir                          ; If no remainder then do a disk read
    inc al                                    ; If there is a remainder increment length by 1 sector

.get_root_dir:
    mov cl, al                                ; Store root dir length in cl
    pop ax                                    ; Retreive starting LBA of a root directory
    mov ah, cl                                ; AH = sectors to read; AL - LBA
    mov bx, buffer                            ; ES:BX is where root dir will be dumped
    call disk_read                            ; AH = num of sectors to read; AL - LBA; ES:BX - memo to dump

.done:
    ret





;===============================| Search file record in the root directory |=============================== 
; Input:
;   - SI = address of a file name (e.g., "STAGE1  BIN")
;   - DI = addres of the root directory (e.g. , 0x7E00)
;   - BX = 0 which will be used as a loop counter (i.e. comparing to bdb_dir_entries_count)
;
; Output (success):
;   - AX = first file cluster (e.g., 0x02) of the file
;   - Clobbers: SI, DI (might not be an exhaustive list)
;
; Output (fail)
;   - System halted since either stage1 or kernel not found
;=========================================================================================================

search_file:
    
    ; Compare root dirctory record to the file name
    push di                                   ; Preserve offset to root dir record before cmpsb auto incremetns it
    push si                                   ; Preserve file name address before cmpsb auto incremetns it
    mov cx, 11                                ; Set comparison counter to 11 bytes (8 for filename + 3 for file format)
    repe cmpsb                                ; Compare all 11 bytes at si:di
    pop si                                    ; Retrieve original file name address
    pop di                                    ; Retrieve original root dir entery offset
    je .file_found                            ; ZF = 1 on success. DI will contain address of the first file name character

    ; Move to the next root direcotory record
    add di, 32                                ; Go to next record in root folder (+32 bytes) 
    inc bx                                    ; Save the number of records that were searched 
    cmp bx, [bdb_dir_entries_count]           ; Compare to the maximum number of records possible for a root directory
    jl search_file                            ; Drop through to file_not_found if all records searched. Otherwise continue

.file_not_found:
    mov si, msg_file_not_found                
    call print
    jmp halt                                  ; Halt. Boot is not possible since either stage1 or kernel not found

.file_found:
    mov si, msg_file_found
    call print
    mov ax, [di+26]                           ; Save starting cluster of the file
    ret





; ==============================|    Load file   |===============================
; Important:
;   - Index of the cluster in the FAT12 table tells you what blocks you need to get from disk (n+31)
;   - To see if your file spans other clusters you need to decode the FAT12 chain marker 
;   - This chain marker is located in the FAT12 record for the cluster
;   - To find it, you need to calculate an offset to it (index * 3 / 2)
;   - Then deal with 1.5 byte even&odd business
;
; Input:
;   AX = starting cluster
;   SI = FAT12 address
;   ES:BX = destination buffer
; Clobbers: AX, BX, CX, DX, SI
; TODO: Confirm that looking at whether the offset is even/ odd (n*3/2) is the same as just n/2
; ================================================================================

load_file:
.load_next_cluster:
    
    ; Load the cluster into memory (starting cluster if its the first time)
    push ax                                   ; Preserve cluster number to calculate offset to its FAT12 record
    add ax, 31                                ; Cluster number -> LBA conversion for floppy
    mov ah, 1                                 ; Number of sectors we'll read (since 1 cluster = 1 sector)
    call disk_read                            ; AH = num of sectors to read; AL - LBA; ES:BX - memo to dump
    add bx, [bdb_bytes_per_sector]            ; Increment offset for the next cluster (BX since it is used in disk read)

    ; Calculate byte offset for the FAT12 record of the CURRENT cluster: (cluster index * 3)/2 = 3
    pop ax                                    ; Restore cluster number
    mov cx, 3
    mul cx
    mov cx, 2
    div cx                                    ; Store offset to the current FAT12 cluster in AX

    ; Save FAT12 chain marker for the CURRENT cluster
    add si, ax                                ; Add the offset for the next FAT entery to look at
    mov ax, [ds:si]                           ; Move next FAT entery to ax (i.e., 2 bytes instead of 1.5 bytes)

    ; Determine if the FAT entry offset is even/odd (placed here to fall through to .odd if not even)
    or dx,dx                       ; dx stores the remaider from division => hacky way of doing cmp dx, 0 
    jz .even

.odd:
    shr ax, 4                      ; Get upper 12 bits (i.e., shift right)
    jmp .interpret_chain_marker

.even:
    and ax, 0x0FFF                 ; Get lower 12 bits

.interpret_chain_marker:
    cmp ax, 0x0FF8                 ; Check for end-of-chain marker (0xFF8-0xFFF)
    jae .read_finished
    jmp .load_next_cluster         ; Do the same for the cluster number in AX

.read_finished:
    ret