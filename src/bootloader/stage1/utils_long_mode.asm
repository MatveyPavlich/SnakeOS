; ===========================| Print_64_bits |============================
; Input:
;   - RDI = pointer to the string (should be 64 bits register since paging is on)
;   - PRINT_STRING_POSSITION = byte for the string start (will be moved to RDI)
; Output:
;   - Save next address to print the string into PRINT_STRING_POSSITION
;   - RDI is cleaned to 0. Everything else in untouched
; ==============================================================


%define RED_ON_BLACK    0x0C
%define BYTES_PER_CHAR  2

print_64_bits:
    push rsi                                  ; Preserve rsi that will be used for video memory pointer later
    push rax                                  ; Preserve rax that will be used for arithmetic later
    xor rax, rax
    xor rsi, rsi
    mov esi, [PRINT_STRING_POSSITION]         ; PRINT_STRING_POSSITION is 32-bit (since created in protected mode)
    push rsi                                  ; Save original PRINT_STRING_POSSITION to add +160 for printing on the next line 

.print_loop:
    mov al, [rdi]                             ; Load next character (since paging is enabled => 64 bit addresses)
    cmp al, 0                                 ; Check if it is the end of the string
    je .done

    mov ah, RED_ON_BLACK                      ; Attribute: red on black
    mov [rsi], ax                             ; Write character and attribute
    inc rdi                                   ; Increment string pointer to the next character
    add rsi, BYTES_PER_CHAR                   ; Next position in video memory
    jmp .print_loop

.done:
    pop rsi                                   ; 
    add rsi, 160                              ; Screen is 80 characters long => 80*2 = 160 for video memory for the next line
    mov [PRINT_STRING_POSSITION], rsi         ; Preserve next video memory to print on
    mov rdi, 0
    pop rax
    pop rsi
    ret
