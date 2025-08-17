ASM=nasm
SRC_DIR=src
BUILD_DIR=build
CFLAGS = -ffreestanding -m64 -nostdlib -O0 -Wall -g -Isrc/kernel/intf
LDFLAGS=-T $(SRC_DIR)/kernel/impl/kernel.ld -nostdlib -z max-page-size=0x200000

# Build floppy image with bootloader in sector 0
floppy_image: $(BUILD_DIR)/main.img

$(BUILD_DIR)/main.img: $(BUILD_DIR)/main.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=2880
	dd if=$(BUILD_DIR)/main.bin of=$(BUILD_DIR)/main.img seek=0 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/main.img bs=512 seek=1 conv=notrunc

# Bootloader (main.asm)
$(BUILD_DIR)/main.bin: $(SRC_DIR)/bootloader/main.asm
	$(ASM) $(SRC_DIR)/bootloader/main.asm -f bin -o $(BUILD_DIR)/main.bin

# === Kernel ===
kernel: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kmain.o $(SRC_DIR)/kernel/impl/kernel.ld $(BUILD_DIR)/kprint.o
	$(LD) $(LDFLAGS) -o $@ $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kmain.o $(BUILD_DIR)/kprint.o
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	objcopy -O binary $< $@

$(BUILD_DIR)/kernel_entry.o: $(SRC_DIR)/kernel/kernel_entry.asm
	$(ASM) -f elf64 $< -o $@

$(BUILD_DIR)/kmain.o: $(SRC_DIR)/kernel/impl/kmain.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kprint.o: $(SRC_DIR)/kernel/impl/kprint.c
	$(CC) $(CFLAGS) -c $< -o $@


# # Kernel
# $(BUILD_DIR)/kernel_entry.bin: $(SRC_DIR)/kernel/kernel_entry.asm
# 	$(ASM) $(SRC_DIR)/kernel/kernel_entry.asm -f bin -o $(BUILD_DIR)/kernel_entry.bin

# Run OS without GDB debug
run: floppy_image
	qemu-system-x86_64 -fda $(BUILD_DIR)/main.img

debug:
	./debug.sh
# Clean
clean:
	rm -rf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.img
