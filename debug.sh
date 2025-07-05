#!/bin/bash
PROJECT_DIR=~/repos/MatthewOS

# Move terminal window and QEMU into workspace 4
hyprctl dispatch exec "[workspace 4 silent] kitty --title QEMU_WINDOW bash -c 'cd $PROJECT_DIR && qemu-system-i386 -fda ./build/main.img -debugcon stdio -s -S; exec bash'"

sleep 0.5

# Execute .gdbinit script
gdb -q -ex "set pagination off" -x "$PROJECT_DIR/.gdbinit"
