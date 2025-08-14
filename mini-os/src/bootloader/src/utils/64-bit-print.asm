bits 64

VIDEO_MEMORY    equ 0xb8000
RED_ON_BLACK    equ 0x0C
BYTES_PER_CHAR  equ 2

print_string_64:
    push rdi
    push rsi
    push rax

    mov rdi, VIDEO_MEMORY         ; Start writing at top-left corner of screen

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
    pop rax
    pop rsi
    pop rdi
    ret
