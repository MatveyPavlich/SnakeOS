set architecture i8086
set disassembly-flavor intel
target remote localhost:1234
layout asm
break *0x7c00
continue
stepi