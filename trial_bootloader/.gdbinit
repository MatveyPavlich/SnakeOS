set architecture i386:x86-64        
set disassembly-flavor intel
target remote localhost:1234
layout asm
break *0x7c00
continue
stepi
br* 0x7c17
continue
stepi
stepi
stepi
stepi
stepi
stepi