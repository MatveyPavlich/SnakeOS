OUTPUT_FORMAT("elf64-x86-64")
ENTRY(kernel_entry)
SECTIONS {
    STACK_TOP = 0x80000;
    . = 0x90000;
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
