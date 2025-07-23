bits 64

RED_ON_BLACK    equ 0x0C
BYTES_PER_CHAR  equ 2

print_string_64:
    push rdi
    push rsi
    push rax
    xor rax, rax
    xor rdi, rdi
    mov edi, [PRINT_STRING_POSSITION]         ; Start writing at top-left corner of screen (0x80280)
    push rdi

.print_loop:
    mov al, [rsi]                 ; Load next character
    cmp al, 0                     ; End of string?
    je .done

    mov ah, RED_ON_BLACK          ; Attribute: red on black
    mov [rdi], ax                 ; Write character and attribute
    add rsi, 1                    ; Next character in string
    add rdi, BYTES_PER_CHAR       ; Next position in video memory
    jmp .print_loop

.done:
    pop rdi
    add rdi, 160
    mov [PRINT_STRING_POSSITION], rdi
    pop rax
    pop rsi
    pop rdi
    ret
