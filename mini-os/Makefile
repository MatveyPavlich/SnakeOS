ASM=nasm
SRC_DIR=src
BUILD_DIR=build

# Build floppy image with bootloader in sector 0
floppy_image: $(BUILD_DIR)/main.img

$(BUILD_DIR)/main.img: $(BUILD_DIR)/main.bin $(BUILD_DIR)/kernel_entry.bin
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=2880
	dd if=$(BUILD_DIR)/main.bin of=$(BUILD_DIR)/main.img seek=0 conv=notrunc
	dd if=$(BUILD_DIR)/kernel_entry.bin of=$(BUILD_DIR)/main.img bs=512 seek=1 conv=notrunc

# Assemble stage0 (main.asm)
$(BUILD_DIR)/main.bin: $(SRC_DIR)/bootloader/main.asm
	$(ASM) $(SRC_DIR)/bootloader/main.asm -f bin -o $(BUILD_DIR)/main.bin

# Kernel
$(BUILD_DIR)/kernel_entry.bin: $(SRC_DIR)/kernel/kernel_entry.asm
	$(ASM) $(SRC_DIR)/kernel/kernel_entry.asm -f bin -o $(BUILD_DIR)/kernel_entry.bin

# Run OS without GDB debug
run: floppy_image
	qemu-system-x86_64 -fda $(BUILD_DIR)/main.img

debug:
	./debug.sh
# Clean
clean:
	rm -rf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.img
