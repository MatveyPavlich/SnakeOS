[org 0x7C00]
[bits 16]

main:

    ; Set memory segments to 0x0000
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov si, MSG_REAL_MODE
    call print
    call ensure_a20             ; Make sure A20 is enabled

    call switch_to_pm           ; Switch to the protected mode
    hlt                         ; In theory should never reach here
    jmp $

switch_to_pm:
    cli                         ; Disable BIOS interrupts (0x7c17)
    lgdt [gdt_descriptor]       ; Load the GDT descriptor
    mov eax, cr0
    or eax, 0x1                 ; Set 32-bit mode bit in cr0
    mov cr0, eax
    jmp dword CODE_SEG:start_pm ; far jump by using a different segment


%include "./src/utils/boot_sect_print.asm"
%include "./src/utils/ensure_a20.asm"
%include "./src/utils/gdt.asm"
; MSG_REAL_MODE db "Started in 16-bit real mode", 0xD, 0xA, 0x00
; A20_FAILED db "A20 couldn't be enabled. System halted", 0xD, 0xA, 0x00
MSG_REAL_MODE db "A", 0xD, 0xA, 0x00
A20_FAILED db "B", 0xD, 0xA, 0x00

; ============================ Protected mode ==============================

[bits 32]
start_pm:
    mov esi, MSG_PROT_MODE                          ; 0x7cf6 in gdb
    call print_string_pm
    ; xor eax, eax
    ; xor ebx, ebx
    ; xor ecx, ecx
    ; xor edx, edx
    ; call check_CPUID
    ; call check_extended_functions
    ; call check_long_mode_support 
    call set_up_paging
    call enable_paging
    ; jmp dword CODE_SEG:Realm64
    jmp dword LONG_CODE_SEG:Realm64

%include "./src/utils/32bit-print.asm"
%include "./src/utils/long_mode.asm"
; %include "./src/utils/cpuid.asm"
MSG_PROT_MODE db "Loaded 32-bit protected mode", 0x00



bits 64

VGA_TEXT_BUFFER_ADDR equ 0xb8000

Realm64:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov byte [VGA_TEXT_BUFFER_ADDR], 0x36
    mov byte [VGA_TEXT_BUFFER_ADDR + 1], 0x0C
    mov byte [VGA_TEXT_BUFFER_ADDR + 2], 0x34
    mov byte [VGA_TEXT_BUFFER_ADDR + 3], 0x0C

    hlt
    jmp $

; %include "./src/utils/64-bit-print.asm"
; str_hello db "Welcome to long mode, baby -_-!", 0


times 510-($-$$) db 0
dw 0xaa55
