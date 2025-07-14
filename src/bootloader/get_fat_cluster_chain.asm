; ==============================| Load stage 1 into RAM |============================== 

loadKernelLoop:
    
    ; Load stage1 cluster into RAM
    mov ax, [stage1_cluster]
    add ax, 31                     ; Cluster number -> LBA conversion
    mov ah, 1                      ; Number of sectors we'll read
    call disk_read
    add bx, [bdb_bytes_per_sector] ; Increment offset to load the next cluster (gdb 0x7ccb)

    ; Find next pointer in FAT
    mov ax, [stage1_cluster] ; (kernel cluster * 3)/2 cause every entery is 1.5B
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx,dx
    jz even

odd:
    shr ax,4
    jmp nextClusterAfter
even:
    and ax, 0x0FFF               ; 

nextClusterAfter:
    cmp ax, 0x0FF8
    jae readFinish

    mov [stage1_cluster], ax
    jmp loadKernelLoop

readFinish:
    mov dl, [ebr_drive_number]
    mov ax, stage1_load_segment
    mov ds,ax
    mov es,ax
    jmp stage1_load_segment:stage1_load_offset