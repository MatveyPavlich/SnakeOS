; ==============================| Load cluster chain |============================== 
; Input:
;   AX = starting cluster
;   ES:BX = destination buffer
; Clobbers: AX, BX, CX, DX, SI
; ================================================================================
load_cluster_chain:
.load_next:
    ; Load the cluster into RAM (starting cluster if it is start of the loop)
    mov ax, [stage1_cluster]
    add ax, 31                     ; Cluster number -> LBA conversion for floppy
    mov ah, 1                      ; Number of sectors we'll read
    call disk_read
    add bx, [bdb_bytes_per_sector] ; Increment offset to load the next cluster (bx is used in disk read for memory offset)

    ; Get FAT entry for the CURRENT cluster (we need to interpret it to see if there is a next cluster)
    mov ax, [stage1_cluster]       ; (kernel cluster * 3)/2 cause every entery is 1.5B
    mov cx, 3
    mul cx
    mov cx, 2
    div cx                         ; AX will now store the offset to the current FAT cluster

    mov si, buffer                 ; Move the start of FAT12 table into si (it was loaded @ buffer)
    add si, ax                     ; Add the offset for the next FAT entery to look at
    mov ax, [ds:si]                ; Move next FAT entery to ax (i.e., 2 bytes instead of 1.5 bytes)

    ; Determine if the FAT entry offset is even or odd
    or dx,dx                       ; dx stores the remaider from division => hacky way of doing cmp dx, 0 
    jz .even

.odd:
    shr ax, 4                    ; Get upper 12 bits (i.e., shift right)
    jmp .interpret_chain_marker

.even:
    and ax, 0x0FFF               ; Get lower 12 bits

.interpret_chain_marker:
    cmp ax, 0x0FF8               ; Check for end-of-chain marker (0xFF8-0xFFF)
    jae .read_finished

    mov [stage1_cluster], ax     ; Store new cluster and repeat!
    jmp .load_next

.read_finished:
    ret