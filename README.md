# SnakeOS 🐍🖥️

SnakeOS is a **from-scratch operating system** written in **x86-64 Assembly and C** with the mission of running a 2D Snake game.

The project explores **low-level OS development**, including bootloader design, memory management, graphics, and hardware interfacing, to build a minimalist but fully functional 64-bit OS.

---

## 📜 Project Goals
- Build a custom 64-bit operating system from scratch
- Boot directly into a 2D Snake game without a traditional OS
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
- **Language:** x86 Assembly (NASM) + C
- **Target Architecture:** x86-64
- **Build Tools:** `nasm`, `gcc`, `ld`, `make`
- **Emulator:** QEMU (primary), Bochs (optional)
- **Disk Format:** FAT12 (floppy image)

---

## 📂 Repository Structure
