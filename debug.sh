#!/bin/bash
PROJECT_DIR=~/repos/SnakeOS
# QEMU_BIN=~/qemu-patched/bin/qemu-system-i386 # For real mode debugging (make sure to adjust gdb "set architecture" line)
# QEMU_BIN=qemu-system-i386                      # For protected mode debuggin (not patched, meaning no segment:offset)
QEMU_BIN=qemu-system-x86_64                  # For long mode (x86_64 architecture)

# Move terminal window and QEMU into workspace 4
hyprctl dispatch exec "[workspace 4 silent] kitty --title QEMU_WINDOW bash -c 'cd $PROJECT_DIR && $QEMU_BIN -fda ./build/main.img -debugcon stdio -s -S; exec bash'"

sleep 0.5

# Execute .gdbinit script
gdb -q -ex "set pagination off" -x "$PROJECT_DIR/.gdbinit"



# Real mode:
#   - QEMU_BIN: ~/qemu-patched/bin/qemu-system-i386 (has addresses in segment:offset)
#   - .gdbinit (start): set architecture i8086

# Protected mode:
#   - QEMU_BIN: qemu-system-i386 (what addressing would it have? 32-bit instruction set)
#   - .gdbinit (start): set architecture i386

# Long mode:
# Protected mode:
#   - QEMU_BIN: qemu-system-x86_64 (what addressing would it have? 64-bit instruction set)
#   - .gdbinit (start): set architecture i386:x86-64
# The moment you run "QEMU_BIN=qemu-system-x86_64" you have to "set architecture i386:x86-64" to debug with GDB
# You can't look at protected mode (32 bits) on "QEMU_BIN=qemu-system-x86_64" with "set architecture i386"