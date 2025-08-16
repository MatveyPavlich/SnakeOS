org 0x7C00
bits 16


; ============================ Real mode ==============================

main:

    ; Set memory segments to 0x0000
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Debug message
    mov si, MSG_REAL_MODE
    call print
    ; call ensure_a20                           ; Make sure A20 is enabled (removed to save some space)
    call disk_read

    ; Switch to protected mode
    cli                                       ; Disable BIOS interrupts (0x7c17)
    lgdt [gdt_descriptor]                     ; Load the GDT descriptor
    mov eax, cr0
    or eax, 0x1                               ; Set 32-bit mode bit in cr0
    mov cr0, eax                              ; Move new value into cr0
    jmp dword CODE_SEG:start_pm               ; Far jump by using a different segment
    hlt                                       ; In theory should never reach here
    jmp $

%include "./src/bootloader/utils/16-bit-utils.asm"
; %include "./src/bootloader/utils/ensure_a20.asm"
%include "./src/bootloader/utils/gdt.asm"
MSG_REAL_MODE db "RM", 0xD, 0xA, 0x00





; ============================ Protected mode ==============================

bits 32

start_pm:    
    
    ; Debugging
    mov word [VGA_TEXT_BUFFER_ADDR], 0x0C50           ; Print P (for Protected)
    mov word [VGA_TEXT_BUFFER_ADDR + 2], 0x0C4D       ; Print M (for Mode)
    ; mov esi, MSG_PROT_MODE                          ; Verbose debugging
    ; call print_string_pm                            ; Verbose debugging

    ; Clean general purpose registers from print_string_pm
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    ; ; See if long mode supported (Commented out due to not enough space)
    ; call check_CPUID
    ; call check_extended_functions
    ; call check_long_mode_support 

    ; Enable long mode
    call set_up_paging
    call enable_paging
    jmp dword LONG_CODE_SEG:start_lm   ; You must have a far jump for some reason...
    ; ; jmp start_lm                     ; Will not work ???
    ; ; jmp dword CODE_SEG:start_lm      ; Will not work ???
    ; ; jmp dword null_descriptor:start_lm ; Will not work

%include "./src/bootloader/utils/long_mode.asm"
; %include "./src/bootloader/utils/cpuid.asm"

; Verbose debugging
; %include "./src/utils/32bit-print.asm"
; MSG_PROT_MODE db "Loaded 32-bit protected mode", 0x00


bits 64

%define KERNEL_LOAD_OFFSET 0x90000
VGA_TEXT_BUFFER_ADDR equ 0xb8000

start_lm:
    ; mov ax, 0
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax
    ; mov ss, ax

    mov edi, 0xB8000                          ; Start of VGA text buffer
    mov ecx, 80 * 25                          ; Number of characters on screen
    mov ax, 0x0720                            ; ' ' (space) with gray-on-black attribute
    rep stosw                                 ; Fill ECX words (AX) into [EDI]

    ; mov si, str_hello                       ; Verbose debugging
    ; call print_string_64                    ; Verbose debugging
    mov word [VGA_TEXT_BUFFER_ADDR], 0x0C4C
    mov word [VGA_TEXT_BUFFER_ADDR + 2], 0x0C4D
    mov rsp, 0x90000                          ; Set stack to grow below the code
    jmp KERNEL_LOAD_OFFSET
    hlt
    jmp $

; %include "./src/utils/64-bit-print.asm"
; str_hello db "Long mode", 0


TIMES 510-($-$$) db 0
dw 0xAA55
