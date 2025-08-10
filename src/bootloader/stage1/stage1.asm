;======================================================================================
; Stage1 of the bootloader for SnakeOS is loaded at 0x8000:0000, with es = ds = 0x8000.
; It enables A20 line and creats the Global Descriptor Table (GDT) to then enter the
; protected mode. Once this is done, it checks for the long mode availability. If it  
; is not supported the the system is halted. If it is, then paging is enabled and 
; long mode is entered. After that, jump is made to the kernel.bin loaded by stage0.
; This jump works since first 1 GiB of virtual addresses mapped to exactly the same
; physical addresses.
;======================================================================================

org 0x80000                                   ; For assembler to organise the code
bits 16                                       ; For assembler to know that should be in 16 bits

enter_stage1:
    
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
    jmp dword CODE_SEG:start_protected_mode   ; Far jump by using the segment from GDT


%include "./src/bootloader/shared_utils.asm"
%include "./src/bootloader/stage1/utils_real_mode.asm"
MSG_STAGE1: db "Stage1 live, do you copy? Pshh... Pshh...", 0x0D, 0x0A, 0x00





; ============================ Protected mode ==============================
; - Check long mode support
; - Set up and enable paging
; - Enter long mode
; ==========================================================================

bits 32

start_protected_mode:
    
    ; Move correct GDT index into segment registers (ds=0x8000 => no such entry in GDT)
    mov ax, DATA_SEG                          ; 0x8010f
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Reset stack to grow down before the code
    mov ebp, 0x80000
    mov esp, ebp

    ; Inform protected mode is entered
    mov esi, MSG_PROT_MODE
    call print_32_bits                        ; ESI = pointer to the message; PRINT_STRING_POSSITION = video memory address
    xor esi, esi

    ; Check if long mode is supported
    call check_CPUID                          ; Esure CPUID instruction is supported
    call check_extended_functions             ; Esure extended functions are supported
    call check_long_mode_support              ; Ensure long mode is supported via extended function

    ; Enable long mode
    call set_up_paging                        ; Create page tables
    call enable_paging                        ; Enable long mode
    mov eax, [PRINT_STRING_POSSITION]
    xor ecx, ecx                              ; Garbaging ecx somewhere 
    jmp dword LONG_CODE_SEG:start_long_mode   ; You must have a far jump for some reason... (0x80143)
    hlt
    jmp $

    ; jmp start_lm                            ; Will not work ???
    ; jmp dword CODE_SEG:start_lm             ; Will not work ???
    ; jmp dword null_descriptor:start_lm      ; Will not work


%include "./src/bootloader/stage1/utils_protected_mode.asm"
MSG_PROT_MODE          db "MODE ENTERED: Protected", 0x00
PRINT_STRING_POSSITION dd 0xb8000


bits 64

start_long_mode:
    
    ; Set segments to zero to make sure when they are used they don't have garbage 
    mov [PRINT_STRING_POSSITION], eax
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Inform long mode was entabled
    mov rdi, long_mode_enabled
    call print_64_bits                        ; RDI = string address; PRINT_STRING_POSSITION = video memory address
    
    ; Jump to the kernel_entry.asm
    mov eax, [PRINT_STRING_POSSITION]         ; Save print string position into RAX (was initiated in protected mode => 32 bits, must use eax)
    jmp kernel_load_offset                    ; Jump to the keranel loaded by stage1. Address is the same since first 1 GiB was identity mapped (physical = virtual address)

    hlt
    jmp $

%include "./src/bootloader/stage1/utils_long_mode.asm"
long_mode_enabled db "MODE ENTERED: Long", 0
kernel_load_offset equ 0x90000                ; Stage0 loaded kernel.bin at this offset (stayed the same since)
                                              