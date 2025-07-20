# Start in real mode
# set architecture i386:x86-64 # For long mode debugging
# set architecture i8086       # For real mode debugging
# For protected mode debugging
set architecture i386        
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

# Move into protected mode
stepi