# === Variables ===
ASM=nasm
CC=gcc
LD=ld
BUILD_DIR=build

CFLAGS = -ffreestanding -m64 -nostdlib -O0 -Wall -g -Iinclude \
	 -fno-stack-protector -fno-stack-check
# -fno-stack-protector -fno-stack-check prevent GCC inserting hidden runtime
# dependencies into a freestanding kernel.
# Note! -O0 removes optimisations to easier debug
# Commit a4214a854972e04ffb38ac2af53b898a47688990: code breaks with "-O2" flag
# (i.e., when compiler starts to optimise the code) 

LDFLAGS=-T kernel.ld -nostdlib -z max-page-size=0x200000

# === Phony targets ===
.PHONY: all floppy_image stage0 stage1 kernel run debug clean


# === Floppy disk ===
floppy_image: $(BUILD_DIR)/main.img
$(BUILD_DIR)/main.img: stage0 stage1 kernel
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=2880
	mkfs.fat -F 12 -n "SNAKEOS" $(BUILD_DIR)/main.img
	dd if=$(BUILD_DIR)/stage0.bin of=$(BUILD_DIR)/main.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/stage1.bin "::stage1.bin"
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/kernel.bin "::kernel.bin"

# === Bootloader ===
stage0: $(BUILD_DIR)/stage0.bin
$(BUILD_DIR)/stage0.bin:
	$(ASM) boot/x86/stage0/stage0.asm -f bin -o $(BUILD_DIR)/stage0.bin

# === Stage 1 ===
stage1: $(BUILD_DIR)/stage1.bin
$(BUILD_DIR)/stage1.bin:
	$(ASM) boot/x86/stage1/stage1.asm -f bin -o $(BUILD_DIR)/stage1.bin

# === Kernel ===
kernel: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kmain.o   \
		         $(BUILD_DIR)/kprint.o $(BUILD_DIR)/gdt.o           \
			 $(BUILD_DIR)/gdt_asm.o $(BUILD_DIR)/util.o         \
			 $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o              \
			 $(BUILD_DIR)/isr_asm.o $(BUILD_DIR)/keyboard.o     \
			 $(BUILD_DIR)/init_ram.o $(BUILD_DIR)/print_clock.o \
			 $(BUILD_DIR)/irq.o $(BUILD_DIR)/irq.o \
			 kernel.ld
	$(LD) $(LDFLAGS) -o $@ $(BUILD_DIR)/kernel_entry.o \
			    $(BUILD_DIR)/kmain.o           \
			    $(BUILD_DIR)/kprint.o          \
			    $(BUILD_DIR)/gdt.o             \
			    $(BUILD_DIR)/gdt_asm.o         \
   			    $(BUILD_DIR)/util.o            \
			    $(BUILD_DIR)/idt.o             \
			    $(BUILD_DIR)/isr.o             \
			    $(BUILD_DIR)/isr_asm.o         \
			    $(BUILD_DIR)/keyboard.o        \
			    $(BUILD_DIR)/init_ram.o        \
			    $(BUILD_DIR)/irq.o             \
			    $(BUILD_DIR)/print_clock.o
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	objcopy -O binary $< $@

$(BUILD_DIR)/kernel_entry.o: boot/x86/kernel_entry.asm
	$(ASM) -f elf64 $< -o $@
$(BUILD_DIR)/gdt_asm.o: arch/x86/gdt.asm
	$(ASM) -f elf64 $< -o $@
$(BUILD_DIR)/isr_asm.o: arch/x86/isr.asm
	$(ASM) -f elf64 $< -o $@

$(BUILD_DIR)/kmain.o: init/kmain.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/kprint.o: kernel/kprint.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/gdt.o: arch/x86/gdt.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/util.o: lib/util.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/idt.o: arch/x86/idt.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/isr.o: arch/x86/isr.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/keyboard.o: drivers/keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/init_ram.o: drivers/init_ram.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/print_clock.o: drivers/print_clock.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/irq.o: kernel/irq.c
	$(CC) $(CFLAGS) -c $< -o $@

# === Run ===
run:
	qemu-system-x86_64 -fda $(BUILD_DIR)/main.img

# === Debug ===
debug:
	./debug.sh

# === Clean ===
clean:
	rm -rf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.img $(BUILD_DIR)/*.o
