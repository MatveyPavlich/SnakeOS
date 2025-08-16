; Code adopted from: osdev.org, Setting Up Long Mode, link: https://wiki.osdev.org/Setting_Up_Long_Mode


; ===================================| check_CPUID |=========================================
; Checks if CPUID is supported by attempting to flip the ID bit (bit 21) in the EFLAGS
; register. If it can be flipped then CPUID is available.
; Input: void
; Output:
;   - EAX = 1 if cpuid is supported. 0 if not.
; ===========================================================================================

EFLAGS_ID equ 1 << 21           ; if this bit can be flipped, the CPUID instruction is available

check_CPUID:
    
    ; Move EFLAGS to EAX, flip bit 21, load EFLAGS back to flags register 
    pushfd                                        ; Save current EFLAGS onto the stack
    pop eax                                       ; Move saved EFLAGS into EAX
    mov ecx, eax                                  ; Save original EFLAGS in ECX for later comparison (+ restoration)
    xor eax, EFLAGS_ID                            ; Flip bit 21 in EAX (i.e. EFLAGS)
    push eax                                      ; Save the modified FLAGS value to the stack
    popfd                                         ; Load that value into the actual FLAGS register

    ; Check if bit 21 was sucessfully flipped in EFLAGS
    pushfd                                        ; Save the FLAGS register (to push into EAX)
    pop eax                                       ; Load the possibly modified FLAGS back into EAX

    ; Restore EFLAGS to its original value before comparing
    push ecx
    popfd

    ; Compare old vs new flags
    xor eax, ecx                                  ; EAX != ECX if the bit was successfully flipped. CPUID is supported.
    jnz .supported

    .not_supported:
        xor eax, eax
        mov al, 0
        ret

    .supported:
        xor eax, eax
        mov al, 1
        ret


; ============================| check_extended_functions |===================================
; Check if CPUID supports extended functions (that detect the presence of long mode). If not,
; then CPU likaly does not support the long mode since it can't report on its support
; Input: void
; Output:
;   - EAX = 1 if CPUID supports extended functions. 0 if not.
; ===========================================================================================

CPUID_EXTENSIONS equ 0x80000000 ; returns the maximum extended requests for cpuid
CPUID_EXT_FEATURES equ 0x80000001 ; returns flags containing long mode support among other things

check_extended_functions:

    mov eax, CPUID_EXTENSIONS
    cpuid
    cmp eax, CPUID_EXT_FEATURES
    jb .extended_functions_not_supported             

    .extended_functions_supported:
        xor eax, eax
        mov eax, 1
        ret
    
    .extended_functions_not_supported:
        xor eax, eax
        mov eax, 0
        ret
    



CPUID_EDX_EXT_FEAT_LM equ 1 << 29   ; if this is set, the CPU supports long mode

check_long_mode_support:

    mov eax, CPUID_EXT_FEATURES
    cpuid
    test edx, CPUID_EDX_EXT_FEAT_LM
    jz .long_mode_not_supported

    .long_mode_supported:
        xor eax, eax
        mov eax, 1
        ret
    
    .long_mode_not_supported:
        xor eax, eax
        mov eax, 0
        ret
