ASM=nasm
SRC_DIR=src
BUILD_DIR=build


# Floppy disk
floppy_image: $(BUILD_DIR)/main.img                                                   # Alias for commandline (i.e., make floppy_image)
$(BUILD_DIR)/main.img: stage0 stage1 kernel                                              # Target
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=2880
	mkfs.fat -F 12 -n "SNAKEOS" $(BUILD_DIR)/main.img
	dd if=$(BUILD_DIR)/stage0.bin of=$(BUILD_DIR)/main.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/stage1.bin "::stage1.bin"
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/kernel_entry.bin "::kernel.bin"
 
# Bootloader 
stage0: $(BUILD_DIR)/stage0.bin 											      # Alias for commandline (i.e., make bootloader)
$(BUILD_DIR)/stage0.bin:                                                          # Target
	$(ASM) $(SRC_DIR)/bootloader/stage0/stage0.asm -f bin -o $(BUILD_DIR)/stage0.bin 

# Stage 1
stage1: $(BUILD_DIR)/stage1.bin
$(BUILD_DIR)/stage1.bin:
	$(ASM) $(SRC_DIR)/bootloader/stage1/stage1.asm -f bin -o $(BUILD_DIR)/stage1.bin


# Kernel 
kernel: $(BUILD_DIR)/kernel_entry.bin                                                       # Alias for commandline (i.e., make kernel)
$(BUILD_DIR)/kernel_entry.bin:                                                              # Target
	$(ASM) $(SRC_DIR)/kernel/kernel_entry.asm -f bin -o $(BUILD_DIR)/kernel_entry.bin

# Run OS without gdb debug
run:
	qemu-system-i386 -fda build/main.img

# This one doesn't build .iso! Run chmod +x script.sh to give script premission to execute.
debug:
	./debug.sh

clean:
	rm -rf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.img
