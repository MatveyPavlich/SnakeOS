org 0x0000                                    ; For assembler to organise the code where first address is 0x7E00
bits 16                                       ; For assembler to know that should be in 16 bits

; Loaded at 0x9000:0000 in RAM
; es = ds = 0x9000
; bx = offset = 0x0000
main:
    mov si, stage1_message
    call print
    jmp halt

%include "./src/bootloader/utils/utils.asm"


stage1_message:  DB "Stage1 live, do you copy? Pshh... Pshh...", 0x0D, 0x0A, 0x00