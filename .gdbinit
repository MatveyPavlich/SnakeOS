set architecture i386:x86-64
set disassembly-flavor intel
target remote localhost:1234
symbol-file build/kernel.elf
add-symbol-file build/kernel.elf 0x90000
layout asm
break *0x7c00
continue
stepi
br* kmain
continue

define restart
    disconnect
    shell pkill -f qemu-system-x86_64
    shell sleep 0.5 && ./debug.sh
end
