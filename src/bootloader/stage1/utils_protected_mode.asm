; ===========================| Print_32_bits |============================
; Input:
;   - ESI = pointer to the string
;   - PRINT_STRING_POSSITION = word for the string start (will be moved to RDI)
; Output:
;   - Save next address to print the string into PRINT_STRING_POSSITION
;   - ESI is cleaned to 0. Everything else in untouched
; ==============================================================

RED_ON_BLACK    equ 0x0C
BYTES_PER_CHAR  equ 2

print_32_bits:
    
    ; Preserve registers to be used
    push eax
    push edx
    xor eax, eax
    xor edx, edx

    ; get video memory position for the string
    mov edx, [PRINT_STRING_POSSITION]         ; Video memory position
    push edx                                  ; Save original PRINT_STRING_POSSITION to add +160 for printing on the next line

.print_string_pm_loop:

    ; Get the next string character into al
    mov al, [esi]                             ; [ebx] is the address of our character
    cmp al, 0                                 ; check if end of string
    je .print_string_pm_done

    ; Save byte into video memory 
    mov ah, RED_ON_BLACK                      ; Add red on black colour to the character
    mov [edx], ax                             ; store character + attribute in video memory
    inc esi                                   ; Move to the next character to print in the string
    add edx, BYTES_PER_CHAR                   ; Move to the next video memory position
    jmp .print_string_pm_loop

.print_string_pm_done:

    ; Save video memory for the next string
    pop edx
    add edx, 160                              ; Increment video memory to the next line
    mov [PRINT_STRING_POSSITION], edx         ; Save next string position

    ; Restore registers to original values
    xor edx, edx
    xor esi, esi
    xor eax, eax
    pop edx
    pop eax
    ret






; ===================================| check_CPUID |=========================================
; Checks if CPUID is supported by attempting to flip the ID bit (bit 21) in the EFLAGS
; register. If it can be flipped then CPUID is available.
; Input: void
; Output:
;   - Print 'SUCCESS' message if supported. 'FAIL' message + halt if not.
; Adopted from: osdev.org, Setting Up Long Mode, https://wiki.osdev.org/Setting_Up_Long_Mode
; ===========================================================================================

EFLAGS_ID equ 1 << 21                             ; if this bit can be flipped, the CPUID instruction is available

check_CPUID:
    
    ; Move EFLAGS to EAX, flip bit 21, load EFLAGS back to flags register 
    pushfd                                        ; Save current EFLAGS onto the stack
    pop eax                                       ; Move saved EFLAGS into EAX
    mov ecx, eax                                  ; Save original EFLAGS in ECX for later comparison (+ restoration)
    xor eax, EFLAGS_ID                            ; Flip bit 21 in EAX (i.e. EFLAGS)
    push eax                                      ; Save the modified FLAGS value to the stack
    popfd                                         ; Load that value into the actual FLAGS register

    ; Check if bit 21 was sucessfully flipped in EFLAGS
    pushfd                                        ; Save the FLAGS register (to push into EAX)
    pop eax                                       ; Load the possibly modified FLAGS back into EAX

    ; Restore EFLAGS to its original value before comparing
    push ecx
    popfd

    ; Compare old vs new flags
    xor eax, ecx                                  ; EAX != ECX if the bit was SUCCESSFULfully flipped. CPUID is supported.
    jnz .supported

    .not_supported:
        mov esi, MSG_CPUID_NOT_SUPP
        call print_32_bits
        hlt
        jmp $

    .supported:
        mov esi, MSG_CPUID_SUPP
        call print_32_bits
        ret

    MSG_CPUID_SUPP db "SUCCESSFUL: CPUID supported. Checking extended functions support...", 0x00
    MSG_CPUID_NOT_SUPP db "ERROR: CPUID not supported. Can't check long mode support. System halted.", 0x00






; ============================| check_extended_functions |===================================
; Check if CPUID supports extended functions (that detect the presence of long mode). If not,
; then CPU likely does not support the long mode since it can't report on its support.
; Input: void
; Output:
;   - Print 'SUCCESS' message if supported. 'FAIL' message + halt if not.
; Adopted from: osdev.org, Setting Up Long Mode, https://wiki.osdev.org/Setting_Up_Long_Mode
; ===========================================================================================

CPUID_EXTENSIONS equ 0x80000000                              ; returns the maximum extended requests for cpuid
CPUID_EXT_FEATURES equ 0x80000001                            ; returns flags containing long mode support among other things

check_extended_functions:

    mov eax, CPUID_EXTENSIONS
    cpuid
    cmp eax, CPUID_EXT_FEATURES
    jb .extended_functions_not_supported             

    .extended_functions_supported:
        mov esi, MSG_EXT_FUNC_SUPP
        call print_32_bits
        ret
    
    .extended_functions_not_supported:
        mov esi, MSG_EXT_FUNC_NOT_SUPP
        call print_32_bits
        hlt
        jmp $

    MSG_EXT_FUNC_SUPP db "SUCCESSFUL: CPUID supports extended functions. Checking long mode support...", 0x00
    MSG_EXT_FUNC_NOT_SUPP db "ERROR: Extended functions not supported by CPUID. Can't check long mode support. System halted.", 0x00






; ============================| check_long_mode_support |===================================
; Check if CPU supports long mode using extended functions of the cpuid instruction.
; Input: void
; Output:
;   - Print 'SUCCESS' message if supported. 'FAIL' message + halt if not.
; Adopted from: osdev.org, Setting Up Long Mode, https://wiki.osdev.org/Setting_Up_Long_Mode
; ===========================================================================================

CPUID_EDX_EXT_FEAT_LM equ 1 << 29   ; if this is set, the CPU supports long mode

check_long_mode_support:

    mov eax, CPUID_EXT_FEATURES
    cpuid
    test edx, CPUID_EDX_EXT_FEAT_LM
    jz .long_mode_not_supported

    .long_mode_supported:
        mov esi, MSG_LONG_SUPP
        call print_32_bits
        ret
    
    .long_mode_not_supported:
        mov esi, MSG_LONG_NOT_SUPP
        call print_32_bits
        hlt
        jmp $

    MSG_LONG_SUPP db "SUCCESSFUL: Long mode supported. Setting up paging...", 0x00
    MSG_LONG_NOT_SUPP db "ERROR: Long mode not supported. System halted.", 0x00






; ===========================| set_up_paging |============================
; Each table is 4 KiB in size (512 * 8 byte entries) located at a multiple
; of 4 KiB (i.e., 0x1000) so that each table would start at offset 0 on each
; page and prevent them spanning > 1 page.
; - Input: void
; - Output:
;   - 'SUCCESS' message on completion of the function
; ========================================================================

page_table_size              equ 4096         ; Size of each table (= 4KiB = 0x1000)
page_table_l4_address        equ 0x1000
page_table_l3_address        equ 0x2000
page_table_l2_address        equ 0x3000

set_up_paging:

    ; Let CPU know where the main page table is
    mov edi, page_table_l4_address
    mov cr3, edi

    ; Clean memory for 4 tables by adding zeros to each byte
    xor eax, eax                              ; Create a 4 byte zero value (0x00000000)
    mov ecx, page_table_size                  ; Set ecx to the table size
    rep stosd                                 ; writes page_table_size of eax value (i.e., page_table_size * 4 bytes)
    mov edi, cr3                              ; reset di back to the beginning of the page table

    ; Create an entry in l4 table (1 entry = up to 512 GiB with 512 different l3 tables)
    mov eax, page_table_l3_address            ; Entry in l4 will point to an l3 table
    or eax, 0b11                              ; present and writable flags
    mov [page_table_l4_address], eax          ; Save entry into l4 table

    ; Create an entry in l3 table (need only 1 to map first GiB)
    mov eax, page_table_l2_address            ; Entry in l3 will point to the l2 table
    or eax, 0b11                              ; present and writable flags
    mov [page_table_l3_address], eax          ; Save entry into l3 table

    ; Fill l2 table with 512 pages of 2 MiB each
    mov ecx, 0
    
    .fill_l2_table_loop:
        mov eax, 0x200000                     ; 2 MiB page
        mul ecx                               ; Find physical address for page start
        or eax, 0b10000011                    ; Add flags (don't forget a 'huge page flag')
        mov [page_table_l2_address + ecx * 8], eax

        inc ecx
        cmp ecx, 512
        jne .fill_l2_table_loop 

        mov esi, MSG_PAGING_SET_UP
        call print_32_bits
        ret
    
    MSG_PAGING_SET_UP db "SUCCESSFUL: Paging set up. Enabling paging ...", 0x00






; ===========================| enable_paging |============================
; This subroutine passes the location of l4 table into cr3, enables Physical 
; Address Extension (PAE) by turing on 5th bit in cr4
; - Input: void
; - Output:
;   - x, y, z registers are clobbered
; ========================================================================

enable_paging:
    
    ; Pass table location to the cpu
    mov eax, page_table_l4_address
    mov cr3, eax
    
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5                  
    mov cr4, eax

    ; Enable long mode
    mov ecx, 0xC0000080                       ; Move this magic value into ecx
    rdmsr                                     ; ReaD the value from the Model Specific Register (RDMSR)
    or eax, 1 << 8                            ; Enable long mode flage at bit number 8
    wrmsr                                     ; WRite the value into Model Specific Register (WRMSR)

    ; enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    mov esi, MSG_PAGING_ENABLED
    call print_32_bits

    ret

    MSG_PAGING_ENABLED db "SUCCESSFUL: Paging enabled. Jumping into long mode...", 0x00