# === Variables ===
ASM=nasm
CC=gcc
LD=ld
SRC_DIR=src
BUILD_DIR=build

CFLAGS = -ffreestanding -m64 -nostdlib -O0 -Wall -g -Isrc/kernel/intf -fno-stack-protector -fno-stack-check
# -fno-stack-protector -fno-stack-check prevent GCC inserting hidden runtime dependencies into a freestanding kernel.

# Note! -O0 removes optimisations to easier debug
# Commit a4214a854972e04ffb38ac2af53b898a47688990: code breaks with "-O2" flag (i.e., when compiler starts to optimise the code) 
LDFLAGS=-T $(SRC_DIR)/kernel/impl/kernel.ld -nostdlib -z max-page-size=0x200000

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
	$(ASM) $(SRC_DIR)/bootloader/stage0/stage0.asm -f bin -o $(BUILD_DIR)/stage0.bin

# === Stage 1 ===
stage1: $(BUILD_DIR)/stage1.bin
$(BUILD_DIR)/stage1.bin:
	$(ASM) $(SRC_DIR)/bootloader/stage1/stage1.asm -f bin -o $(BUILD_DIR)/stage1.bin

# === Kernel ===
kernel: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kmain.o $(BUILD_DIR)/kprint.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/gdt_asm.o $(BUILD_DIR)/util.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/isr_asm.o $(SRC_DIR)/kernel/impl/kernel.ld 
	$(LD) $(LDFLAGS) -o $@ $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kmain.o $(BUILD_DIR)/kprint.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/gdt_asm.o $(BUILD_DIR)/util.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/isr_asm.o
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	objcopy -O binary $< $@

$(BUILD_DIR)/kernel_entry.o: $(SRC_DIR)/kernel/impl/kernel_entry.asm
	$(ASM) -f elf64 $< -o $@
$(BUILD_DIR)/gdt_asm.o: $(SRC_DIR)/kernel/impl/gdt.asm
	$(ASM) -f elf64 $< -o $@
$(BUILD_DIR)/isr_asm.o: $(SRC_DIR)/kernel/impl/isr.asm
	$(ASM) -f elf64 $< -o $@

$(BUILD_DIR)/kmain.o: $(SRC_DIR)/kernel/impl/kmain.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/kprint.o: $(SRC_DIR)/kernel/impl/kprint.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/gdt.o: $(SRC_DIR)/kernel/impl/gdt.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/util.o: $(SRC_DIR)/kernel/impl/util.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/idt.o: $(SRC_DIR)/kernel/impl/idt.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/isr.o: $(SRC_DIR)/kernel/impl/isr.c
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
