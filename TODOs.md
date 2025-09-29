1. Load kernel while still in stage0! - done
2. Jmp into kernel from stage1 - done
2. Create a C file - done (can't install i386 tool chain...)
3. Link together - done

New plan (enter long mode):
1. Detect the presence of the long mode - done
2. Set up paging - done
3. Enter long mode - done

Plan for C:
1. Create IDT - done (might need to set up other essential interrupts such as divide by zero etc)
2. Create keyboard drivers - done
3. Create basic shell interface
4. Create alarm 
5. Create a clock
6. Create file system drivers to be able to read from disk
7. Set up graphics card
8. Do double buffering