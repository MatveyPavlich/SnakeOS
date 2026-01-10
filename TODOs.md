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
- [ ] Create basic shell interface
- [ ] Create file system drivers to be able to read from disk
- [ ] Set up graphics card
- [ ] Do double buffering

### 2025 Christmass work: Refactor, refactor, refactor...
Convert spaghetti code from callbacks, inititation procedures and core ops into
a modular codebase with a dedicated mechanism and code modules.
- [x] Remove IRQ code and PIC initiation from the IDT. It is solely resposible for initiating the idt.
- [x] Create an IRQ core module to act as a policy interface (you'll need to decouple PIC from it).
- [x] Create a separate PIC driver (i8259) that will feed the IRQ module.
- [x] Decouple keyboard code from PIC and PS/2 (i8042) keyboard controllers.
- [x] Create a separate PS/2 driver module to power keyboard core module.

### TODOs
- [x] Separate PIT (i8253) code into a separate driver from timer.c
- [x] Create a console subsystem and offload relevant logic to it from kprint and timer.c
- [x] Create a core timer module from devices/timer.c and migrate it to `/kernel` 
- [ ] Implement tty module with cursor movements.
- [ ] Scope how to solve the  data race in the `keyboard.c` module.
- [ ] Address data race in the `keyboard.c` module.
- [ ] After the console is done scope out how to implement tasks
- [ ] Scope how to implement a log functionality for debugging the OS
- [ ] Refactor the current build system (monolith Makefile)
