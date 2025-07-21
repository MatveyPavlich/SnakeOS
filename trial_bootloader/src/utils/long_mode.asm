; Each table is 4 KiB in size (512 * 8 byte entries)
page_table_size              equ 4096         ; Size of each table (4KiB size, 0x1000 in hex)
page_table_l4_address        equ 0x1000       ; Page Map Level 4 Table address
page_table_l3_address        equ 0x2000       ; Page Directory Pointer Table address
page_table_l2_address        equ 0x3000       ; Page Directory Table address
page_table_l1_address        equ 0x4000       ; Page Table address
; PT_ADDR_MASK      equ 0xffffffffff000         ; Let last 12 bits be for flags
; PT_PRESENT        equ 1                       ; marks the entry as in use (i.e., 0b1)
; PT_READABLE       equ 2                       ; marks the entry as r/w    (i.e., 0b10)
; ENTRIES_PER_PT    equ 512                     ; Enteries in each table
; SIZEOF_PT_ENTRY   equ 8                       ; Size of each entery in a table
; PAGE_SIZE         equ 0x1000                  ; 4 KiB (each page)


; Identity mapping => match a physical address to the same virtual address (should be true for our l4 table since we are moving a pointer to it before paging was enabled)

set_up_paging:

    ; Let CPU know where where the main page table is
    mov edi, page_table_l4_address
    mov cr3, edi

    ; Clean memory for 4 tables by adding zeros to each byte
    xor eax, eax                              ; Create a 4 byte zero value (0x00000000)
    mov ecx, page_table_size                  ; Set ecx to the table size
    rep stosd                                 ; writes page_table_size of eax value (i.e., page_table_size * 4 bytes)
    mov edi, cr3                              ; reset di back to the beginning of the page table

    mov eax, page_table_l3_address
    or eax, 0b11                              ; present and writable flags
    mov [page_table_l4_address], eax

    mov eax, page_table_l2_address
    or eax, 0b11
    mov [page_table_l3_address], eax

    mov ecx, 0
    
    .fill_l2_table_loop:
        mov eax, 0x200000
        mul ecx
        or eax, 0b10000011
        mov [page_table_l2_address + ecx * 8], eax

        inc ecx
        cmp ecx, 512
        jne .fill_l2_table_loop 

        ret


enable_paging:
    
    ; Pass table location to the cpu
    mov eax, page_table_l4_address
    mov cr3, eax
    
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret


