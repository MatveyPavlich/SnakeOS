### Bootloader
- [x] Load kernel and stage1 in stage0
- [x] Jmp into stage1 and then kernel_entry
- [x] Create a C file - done (can't install i386 tool chain...)
- [x] Link together - done
- [x] Jump into C from kernel_entry
- [x] (Extension work) Detect the presence of the long mode
- [x] (Extension work) Setup paging
- [x] (Extension work) Enter long mode to make your OS 64 bits

### Kernel
- [x] Set up IDT with a common stub for IRQs and exceptions
- [x] Create a keyboard driver (for the PS/2 controller)
- [x] Create timer driver (PIT)
- [] Create basic shell interface
- [] Create file system drivers to be able to read from disk
- [] Set up graphics card
- [] Do double buffering
