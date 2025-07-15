print:
    pusha

; while (string[i] != 0) { print string[i]; i++ }
start:
    mov al, [si] ; 'bx' is the base address for the string
    cmp al, 0 
    je done

    ; the part where we print with the BIOS help
    mov ah, 0x0e
    int 0x10 ; 'al' already contains the char

    ; increment pointer and do next loop
    add si, 1
    jmp start

done:
    popa
    ret