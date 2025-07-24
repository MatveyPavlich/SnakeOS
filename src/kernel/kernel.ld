OUTPUT_FORMAT("binary")
ENTRY(kernel_main)
SECTIONS {
    STACK_TOP = 0x80000;
    . = 0x90000;
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
