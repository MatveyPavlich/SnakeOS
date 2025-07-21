; Each table is 4 KiB in size (512 * 8 byte entries)
page_table_size              equ 4096         ; Size of each table (4KiB size, 0x1000 in hex)
page_table_l4_address        equ 0x1000       ; Page Map Level 4 Table address
page_table_l3_address        equ 0x2000       ; Page Directory Pointer Table address
page_table_l2_address        equ 0x3000       ; Page Directory Table address
page_table_l1_address        equ 0x4000       ; Page Table address
PT_ADDR_MASK      equ 0xffffffffff000         ; Let last 12 bits be for flags
PT_PRESENT        equ 1                       ; marks the entry as in use (i.e., 0b1)
PT_READABLE       equ 2                       ; marks the entry as r/w    (i.e., 0b10)
ENTRIES_PER_PT    equ 512                     ; Enteries in each table
SIZEOF_PT_ENTRY   equ 8                       ; Size of each entery in a table
PAGE_SIZE         equ 0x1000                  ; 4 KiB (each page)

set_up_paging:

    ; Let CPU know where where the main page table is
    mov edi, page_table_l4_address
    mov cr3, edi

    ; Clean memory for 4 tables by adding zeros to each byte
    xor eax, eax                              ; Create a 4 byte zero value (0x00000000)
    mov ecx, page_table_size                  ; Set ecx to the table size
    rep stosd                                 ; writes page_table_size of eax value (i.e., page_table_size * 4 bytes)
    mov edi, cr3                              ; reset di back to the beginning of the page table

    ; QWORD is not supported in 32 bits?!
    mov dword [edi], PDPT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE
    mov edi, PDPT_ADDR
    mov dword [edi], PDT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE
    mov edi, PDT_ADDR
    mov dword [edi], PT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE

    ; Prepare registers before the loop to fill the page table
    mov edi, PT_ADDR
    mov ebx, PT_PRESENT | PT_READABLE
    mov ecx, ENTRIES_PER_PT      ; 1 full page table addresses 2MiB
    jmp .set_entry

    ; Enable paging 
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

.set_entry:
    mov [edi], ebx
    add ebx, PAGE_SIZE
    add edi, SIZEOF_PT_ENTRY
    loop .set_entry               ; Set the next entry.



; Access bits
PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0

; Flags bits
GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5

GDT:
    .Null: equ $ - GDT
        dq 0
    .Code: equ $ - GDT
        .Code.limit_lo: dw 0xffff
        .Code.base_lo: dw 0
        .Code.base_mid: db 0
        .Code.access: db PRESENT | NOT_SYS | EXEC | RW
        .Code.flags: db GRAN_4K | LONG_MODE | 0xF   ; Flags & Limit (high, bits 16-19)
        .Code.base_hi: db 0
    .Data: equ $ - GDT
        .Data.limit_lo: dw 0xffff
        .Data.base_lo: dw 0
        .Data.base_mid: db 0
        .Data.access: db PRESENT | NOT_SYS | RW
        .Data.Flags: db GRAN_4K | SZ_32 | 0xF       ; Flags & Limit (high, bits 16-19)
        .Data.base_hi: db 0
    .Pointer:
        dw $ - GDT - 1
        dq GDT


