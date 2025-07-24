; ===========================| Print_64_bits |============================
; Input:
;   - RSI = pointer to the string (should be 64 bits register since paging is on)
;   - PRINT_STRING_POSSITION = byte for the string start (will be moved to RDI)
; Output:
;   - Save next address to print the string into PRINT_STRING_POSSITION
;   - ESI is cleaned to 0. Everything else in untouched
; ==============================================================


RED_ON_BLACK    equ 0x0C
BYTES_PER_CHAR  equ 2

print_64_bits:
    push rsi
    push rsi
    push rax
    xor rax, rax
    xor rsi, rsi
    mov esi, [PRINT_STRING_POSSITION]         ; Start writing at top-left corner of screen (0x80280)
    push rsi                                  ; Save original PRINT_STRING_POSSITION to add +160 for printing on the next line 

.print_loop:
    mov al, [rdi]                             ; Load next character (since paging is enabled => 64 bit addresses)
    cmp al, 0                                 ; Check if it is the end of the string
    je .done

    mov ah, RED_ON_BLACK                      ; Attribute: red on black
    mov [rsi], ax                             ; Write character and attribute
    add rdi, 1                                ; Next character in string
    add rsi, BYTES_PER_CHAR                   ; Next position in video memory
    jmp .print_loop

.done:
    pop rsi
    add rsi, 160                              ; Screen is 80 characters long => 80*2 = 160 for video memory for the next line
    mov [PRINT_STRING_POSSITION], rsi         ; Preserve next video memory to print on
    pop rax
    pop rdi
    pop rsi
    ret
