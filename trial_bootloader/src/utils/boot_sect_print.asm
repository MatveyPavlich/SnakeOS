print:
    pusha

; while (string[i] != 0) { print string[i]; i++ }
start:
    mov al, [si] ; 'bx' is the base address for the string
    cmp al, 0 
    je done
    mov ah, 0x0e ; the part where we print with the BIOS help
    int 0x10     ; 'al' already contains the char
    add si, 1    ; increment pointer and do next loop
    jmp start

done:
    popa
    ret