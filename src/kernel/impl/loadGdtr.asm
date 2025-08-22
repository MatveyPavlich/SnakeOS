global loadGdtr

bits 64

; Inputs: 
;   RDI = pointer to GDT metadata passed thrugh an argument in C
;   RSI = code segment selector (e.g., 0x08, L=1)
loadGdtr:
    
    ; Load GDT metadata into gdt register (gdtr)
    lgdt [rdi]
    
    ; Select kernel data segment
    mov ax, 0x10        
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far return into kernel's CS 
    push 0x8                     ; Changing to 0x18, 0x1B breaks it
    lea rax, [rel .return]
    push rax
    retfq                        ; Do far return

    .return:
        ret
