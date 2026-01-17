# 🐍 SnakeOS

SnakeOS is an **x86-64 learning operating system** with the mission
of running a 2D Snake game. **ARM and RISC-V architectures are not supported**.
Compiler support is **GCC and clang only**.

The project explores **low-level OS development**, including bootloader design,
memory management, graphics, and hardware interfacing, to build a minimalist
but fully functional 64-bit OS.


## How to run

### Prerequisites

Make sure the following tools are installed on your system:

- `nasm` (x86 assembly)
- `gcc` (with x86-64 support)
- `ld`
- `make`
- `qemu-system-x86_64`

#### Arch Linux
```
sudo pacman -S nasm gcc make qemu-system-x86
```

#### Ubuntu / Debian
```
sudo pacman -S nasm gcc make qemu-system-x86
```
#### MacOS (Homebrew)
```
brew install nasm qemu
```
> **Note:**  
> On macOS, the system `gcc` is typically an alias for `clang`.  
> This project uses freestanding C and a custom linker script, so a cross-compiler
> is recommended if you run into build issues.

### Build
From the project root, run:
```
make
```

This will:
- Assemble the bootloader (NASM)
- Compile the kernel (C99)
- Link everything into a bootable image
- Produce a FAT12-formatted floppy disk image

The output image is typically located at: `build/main.img`

---

## 📜 Project Goals
- Build a custom 64-bit operating system from scratch
- Gain hands-on experience with:
  - Bootloaders
  - Memory management & paging
  - Keyboard & input handling
  - Low-level graphics rendering
  - File system basics

---

## ✨ Features

### Operating System
- **Custom Bootloader** — boots from BIOS, switches to 64-bit long mode
- **Protected → Long Mode Transition** — complete GDT setup and paging
- **FAT12 Support** — load files directly from a floppy image
- **Keyboard Driver** — low-level input handling
- **Basic Graphics Mode** — VGA or custom framebuffer drawing
- **Memory Management** — paging tables, identity mapping

### Snake Game
- 2D grid-based gameplay
- Real-time input
- Score tracking
- Game-over & restart logic
- Runs natively on SnakeOS without external dependencies

---

## 🛠️ Tech Stack
- **Language:** x86 Assembly (NASM) + C99
- **Target Architecture:** x86-64
- **Build Tools:** `nasm`, `gcc`, `ld`, `make`
- **Emulator:** QEMU (primary), Bochs (optional)
- **Disk Format:** FAT12 (floppy image)

---

## 📂 Repository Structure
```
SnakeOS/
| arch/
| | x86/
| | | gdt.c
| | | i8259.c
| | | idt.c
| | | gdt.asm
| | | idt.asm
| boot/
| | x86/
| | | stage0/
| | | stage1/
| | | kernel_entry.asm
| | | shared_utils.asm
| drivers/
| | cdev.c
| | console.c
| | i8042.c
| | i8253.c
| | init_ram.c
| | keyboard.c
| | timer.c
| | tty.c
| | vga.c
| include/
| init/
| | kmain.c
| kernel/
| | exception.c
| | irq.c
| | kprint.c
| lib/
| Makefile
```
