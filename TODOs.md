1. Load kernel while still in stage0! - done
2. Jmp into kernel from stage1 - done
2. Create a C file - done (can't install i386 tool chain...)
3. Link together - WIP

New plan (enter long mode):
1. Detect the presence of the long mode - done
2. Set up paging - done
3. Enter long mode - done

Plan for C:
1. Create IDT
2. Create file system drivers to be able to read from disk
3. Set up graphics card
4. Do double buffering