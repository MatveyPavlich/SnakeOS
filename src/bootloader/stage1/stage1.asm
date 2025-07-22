;======================================================================================
; Stage1 of the bootloader for SnakeOS is loaded at 0x8000:0000, with es = ds = 0x8000.
; It enables A20 line and creats a Global Descriptor Table (GDT) to then enter the
; protected mode. Once this is done a jump to the kernel that was loaded by stage0 to
; 0x90000 is performed.
;======================================================================================

org 0x80000                                   ; For assembler to organise the code (0x0000 causes issues)
bits 16                                       ; For assembler to know that should be in 16 bits

main:
    
    ; Inform stage1 is loaded
    mov si, MSG_STAGE1                        ; 0x80000 in gdb
    call print                                ; 0x80006 in gdb to skip the print
    
    ; Make sure A20 is enabled
    call ensure_a20                           ; Input = void             

    ; Clear the screen from stage0 prints
    mov ah, 0x00
    mov al, 0x3
    int 0x10

    ; Enter protected mode
    cli                                       ; Disable BIOS interrupts (0x9000e in gdb)
    lgdt [gdt_descriptor]                     ; Load the GDT descriptor
    mov eax, cr0
    or eax, 0x1                               ; Set 32-bit mode bit in cr0
    mov cr0, eax
    jmp dword CODE_SEG:start_pm               ; far jump by using a different segment


%include "./src/bootloader/shared_utils.asm"
%include "./src/bootloader/stage1/utils.asm"
MSG_STAGE1: db "Stage1 live, do you copy? Pshh... Pshh...", 0x0D, 0x0A, 0x00





; ============================ Protected mode ==============================
; Enter protected mode and jump into kernel that was loaded by stage0
; ==========================================================================

bits 32

start_pm:
    
    ; Set segment registers to correct GDT index (ds=0x9000 => no such entry in GDT)
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Reset stack to grow down before the code
    mov ebp, 0x80000
    mov esp, ebp

    ; Clean the screen from the message
    mov edi, 0xB8000                          ; Start of VGA text buffer
    mov ecx, 80 * 25                          ; Number of characters on screen
    mov ax, 0x0720                            ; ' ' (space) with gray-on-black attribute
    rep stosw                                 ; Fill ECX words (AX) into [EDI]

    ; Inform protected mode is entered
    mov esi, MSG_PROT_MODE
    call print_32_bits                        ; ESI = pinter to the message

    ; Clean general purpose registers from print_string_pm
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    ; See if long mode supported
    call check_CPUID
    call check_extended_functions
    call check_long_mode_support 

    ; Enable long mode
    call set_up_paging
    call enable_paging
    jmp dword LONG_CODE_SEG:start_lm   ; You must have a far jump for some reason...
    ; ; jmp start_lm                     ; Will not work ???
    ; ; jmp dword CODE_SEG:start_lm      ; Will not work ???
    ; ; jmp dword null_descriptor:start_lm ; Will not work
    hlt
    jmp $


%include "./src/bootloader/stage1/print_32_bits.asm"
%include "./src/bootloader/stage1/long_mode.asm"
%include "./src/bootloader/stage1/cpuid.asm"
MSG_PROT_MODE      db "Loaded 32-bit protected mode", 0x00


bits 64

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

    mov si, str_hello                        ; Verbose debugging
    call print_string_64                     ; Verbose debugging
    
    ; Jump to the kernel
    jmp kernel_load_offset

    hlt
    jmp $

%include "./src/bootloader/stage1/64-bit-print.asm"
str_hello db "Welcome to long mode", 0
kernel_load_offset equ 0x90000                ; Stage0 loaded kernel.bin at this offset ????