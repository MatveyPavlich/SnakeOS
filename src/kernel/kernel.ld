OUTPUT_FORMAT("binary")
ENTRY(kernel_main)

SECTIONS {
    . = 0x90000;
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
