global gdt_load
global gdt_load_tss
global idt_load

bits 64

; void gdt_load(struct gdt_metadata *m) - Function to load GDT into the CPU.
; @m:                                     GDT metadata struct defined in gdt.c.
gdt_load:
    
         ; Load GDT metadata into the gdt register (gdtr)
        lgdt [rdi]
    
        ; Reload memo segments to the kernel data segment
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

; void gdt_load_tss(uint_16_t      - Function to load the GDT offset for
;                   gdt_tss_index)   into the CPU.
; @gdt_tss_index:                    Byte offset for the GDT into the TSS
;                                    descriptor;
gdt_load_tss:
        ltr di
        ret


