EFLAGS_ID equ 1 << 21           ; if this bit can be flipped, the CPUID instruction is available


; ===================================| check_CPUID |=========================================
; Checks if CPUID is supported by attempting to flip the ID bit (bit 21) in the EFLAGS
; register. If it can be flipped then CPUID is available.
; Input: void
; Output:
;   - EAX = 1 if cpuid is supported. 0 if not
; ===========================================================================================
check_CPUID:
    pushfd
    pop eax

    ; The original value should be saved for comparison and restoration later
    mov ecx, eax
    xor eax, EFLAGS_ID

    ; storing the eflags and then retrieving it again will show whether or not
    ; the bit could successfully be flipped
    push eax                    ; save to eflags
    popfd
    pushfd                      ; restore from eflags
    pop eax

    ; Restore EFLAGS to its original value
    push ecx
    popfd

    ; if the bit in eax was successfully flipped (eax != ecx), CPUID is supported.
    xor eax, ecx
    jnz .supported
    .notSupported:
        mov ax, 0
        ret
    .supported:
        mov ax, 1
        ret


; CPUID_EXTENSIONS equ 0x80000000 ; returns the maximum extended requests for cpuid
; CPUID_EXT_FEATURES equ 0x80000001 ; returns flags containing long mode support among other things

; .queryLongMode:
;     mov eax, CPUID_EXTENSIONS
;     cpuid
;     cmp eax, CPUID_FEATURES
;     jb .NoLongMode              ; if the CPU can't report long mode support, then it likely
;                                 ; doesn't have it


; CPUID_EDX_EXT_FEAT_LM equ 1 << 29   ; if this is set, the CPU supports long mode

;     mov eax, CPUID_EXT_FEATURES
;     cpuid
;     test edx, CPUID_EDX_EXT_FEAT_LM
;     jz .NoLongMode

; ; Code taken from https://wiki.osdev.org/Setting_Up_Long_Mode