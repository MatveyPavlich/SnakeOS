ASM=nasm
SRC_DIR=src
BUILD_DIR=build


# Floppy disk
floppy_image: $(BUILD_DIR)/main.img                                                   # Alias for commandline (i.e., make floppy_image)
$(BUILD_DIR)/main.img: bootloader kernel                                              # Target
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=2880
	mkfs.fat -F 12 -n "MATTHEWOS" $(BUILD_DIR)/main.img
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/kernel.bin "::kernel.bin"
 
# Bootloader 
bootloader: $(BUILD_DIR)/bootloader.bin 											  # Alias for commandline (i.e., make bootloader)
$(BUILD_DIR)/bootloader.bin:                                                          # Target
	$(ASM) $(SRC_DIR)/bootloader/bootloader.asm -f bin -o $(BUILD_DIR)/bootloader.bin 

# Kernel 
kernel: $(BUILD_DIR)/kernel.bin                                                       # Alias for commandline (i.e., make kernel)
$(BUILD_DIR)/kernel.bin:                                                              # Target
	$(ASM) $(SRC_DIR)/kernel/main.asm -f bin -o $(BUILD_DIR)/kernel.bin

# Run OS without gdb debug
run:
	qemu-system-i386 -fda build/main.img

# This one doesn't build .iso! Run chmod +x script.sh to give script premission to execute.
debug:
	./debug.sh

clean:
	rm -rf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.img
