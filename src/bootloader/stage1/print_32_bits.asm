bits 32                                       ; using 32-bit protected mode



; ===========================| Print_32_bits |============================
; Input:
;   -  
; ==============================================================
print_32_bits:
    pusha                                     ; Preserve all general registers
    mov edx, VIDEO_MEMORY                     ; Move video memory into edx

.print_string_pm_loop:
    mov al, [esi]                             ; [ebx] is the address of our character
    mov ah, RED_ON_BLACK
    cmp al, 0                                 ; check if end of string
    je .print_string_pm_done

    mov [edx], ax                             ; store character + attribute in video memory
    add esi, 1                                ; next char
    add edx, 2                                ; next video memory position

    jmp .print_string_pm_loop

.print_string_pm_done:
    popa
    ret


VIDEO_MEMORY equ 0xb8000                      ; Video memory
RED_ON_BLACK equ 0x0c                         ; Color byte 
