org 0x7E00                                    ; For assembler to organise the code where first address is 0x7E00
bits 16                                       ; For assembler to know that should be in 16 bits

main:
    jmp halt

halt:
    hlt
    jmp halt

