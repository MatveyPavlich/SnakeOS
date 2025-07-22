#!/bin/bash
PROJECT_DIR=~/repos/SnakeOS
QEMU_BIN=~/qemu-patched/bin/qemu-system-i386
# QEMU_BIN=qemu-system-i386

# Move terminal window and QEMU into workspace 4
hyprctl dispatch exec "[workspace 4 silent] kitty --title QEMU_WINDOW bash -c 'cd $PROJECT_DIR && $QEMU_BIN -fda ./build/main.img -debugcon stdio -s -S; exec bash'"

sleep 0.5

# Execute .gdbinit script
gdb -q -ex "set pagination off" -x "$PROJECT_DIR/.gdbinit"
