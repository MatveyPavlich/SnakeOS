; Real mode subroutines used in stage0 and stage1 


; ==============================| Print stuff |============================== 

print:
    push si                                   ; Preserve 
    push ax                                   ; Preserve
    push bx                                   ; Preserve bx and fall trhough to code below

print_loop:
    lodsb                                     ; Load DS:SI byte to al, then increment SI
    or al, al                                 ; Hacky way to avoid CMP al, 0x00
    jz done_print                             ; Finish printing if zero
    mov ah, 0x0E                              ; Set ah to 0x0E to access BIOS teletype print
    mov bh, 0                                 ; Set page number to 0
    int 0x10                                  ; Call BIOS interrup
    jmp print_loop

done_print:
    pop bx                                    ; Get bx value from before print loop
    pop ax                                    ; Get ax value from before print loop
    pop si                                    ; Get si value from before print loop
    ret

; ==============================| Halt |============================== 
halt:
    hlt
    jmp halt