org 0x90000                                   ; For assembler to organise the code where first address is 0x7E00 (having on 0x0000 causes issues)
bits 16                                       ; For assembler to know that should be in 16 bits

; Loaded at 0x9000:0000 in RAM
; es = ds = 0x9000
; bx = offset = 0x0000
; TODO: Load the kernel!!!!
main:
    mov si, stage1_message      ; 0x90000 in gdb
    call print                  ; 0x90006 in gdb to skip the print
    call ensure_a20             ; Make sure A20 is enabled
    call switch_to_pm           ; Switch to the protected mode
    jmp halt                    ; In theory should never reach here

switch_to_pm:
    cli                         ; Disable BIOS interrupts (0x9000e in gdb)
    lgdt [gdt_descriptor]       ; Load the GDT descriptor
    mov eax, cr0
    or eax, 0x1                 ; Set 32-bit mode bit in cr0
    mov cr0, eax
    jmp dword CODE_SEG:start_pm ; far jump by using a different segment


%include "./src/bootloader/shared/utils.asm"
%include "./src/bootloader/stage1/utils/ensure_a20.asm"
%include "./src/bootloader/stage1/utils/gdt.asm"
stage1_message:  db "Stage1 live, do you copy? Pshh... Pshh...", 0x0D, 0x0A, 0x00
MSG_REAL_MODE db "Started in 16-bit real mode", 0xD, 0xA, 0x00
A20_FAILED db "A20 couldn't be enabled. System halted", 0xD, 0xA, 0x00


; ============================ Protected mode ==============================

[bits 32]
start_pm:
    ; Load data segment registers with correct GDT selector
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov ebp, 0x90000       ; Optional: reset stack pointer
    ; Not doing the thing above would mean es=ds=0x9000 => no such entry in GDT
    
    mov esp, ebp
    mov esi, MSG_PROT_MODE
    call print_string_pm
    jmp $

%include "./src/bootloader/stage1/utils/32bit-print.asm"
MSG_PROT_MODE db "Loaded 32-bit protected mode", 0x00