set architecture i386:x86-64
set disassembly-flavor intel
target remote localhost:1234
layout asm
break *0x90000
continue