; ===========================| Print_64_bits |============================
; Input:
;   - ESI = pointer to the string
;   - PRINT_STRING_POSSITION = byte for the string start (will be moved to RDI)
; Output:
;   - Save next address to print the string into PRINT_STRING_POSSITION
;   - ESI is cleaned to 0. Everything else in untouched
; ==============================================================


RED_ON_BLACK    equ 0x0C
BYTES_PER_CHAR  equ 2

print_64_bits:
    push rdi
    push rsi
    push rax
    xor rax, rax
    xor rdi, rdi
    mov edi, [PRINT_STRING_POSSITION]         ; Start writing at top-left corner of screen (0x80280)
    push rdi                                  ; Save original PRINT_STRING_POSSITION to add +160 for printing on the next line 

.print_loop:
    mov al, [rsi]                             ; Load next character (since paging is enabled => 64 bit addresses)
    cmp al, 0                                 ; Check if it is the end of the string
    je .done

    mov ah, RED_ON_BLACK                      ; Attribute: red on black
    mov [rdi], ax                             ; Write character and attribute
    add rsi, 1                                ; Next character in string
    add rdi, BYTES_PER_CHAR                   ; Next position in video memory
    jmp .print_loop

.done:
    pop rdi
    add rdi, 160                              ; Screen is 80 characters long => 80*2 = 160 for video memory for the next line
    mov [PRINT_STRING_POSSITION], rdi         ; Preserve next video memory to print on
    pop rax
    pop rsi
    pop rdi
    ret
