# Something Project v0.2

Something Project is an operating system project made with a custom kernel that works with normal C commands.

For example, the file `kernel.h` works similarly to the C standard library `stdio.h`, allowing the kernel and its functions to be used through simple C code.

The project follows a simple and minimalist Unix-like aesthetic.

---

## 💿 Create the ISO

### 1. Open the Terminal

Enter the project directory:

```bash
cd "Something Project v0.2"

make clean

make```

4. Verify if the ISO Was Created

Use:

ls

If the compilation was successful, the generated files should appear in the project directory.

The bootable ISO is generated inside the "iso/" directory.

If the ISO wasn't created, check the tags of this repository and go to:

Something Project ISO 💿

---

##📁 Project Structure

Something Project v0.2/
├── iso/
├── Makefile
├── boot.o
├── boot.s
├── kernel.bin
├── kernel.c
├── kernel.h
├── kernel.o
├── keyboard.c
├── keyboard.h
├── keyboard.o
├── linker.ld
├── print.c
├── print.o
└── README.md

---

##🧠 Kernel

The kernel is written in C and uses low-level Assembly code for the boot process.

Main kernel files:

- "kernel.c" — Main kernel code
- "kernel.h" — Kernel functions and definitions
- "boot.s" — Boot/Assembly code
- "linker.ld" — Kernel linking configuration
- "print.c" — Printing functions
- "keyboard.c" — Keyboard handling
- "keyboard.h" — Keyboard definitions

---

##⌨️ Keyboard

The "keyboard.c" and "keyboard.h" files handle keyboard input for the operating system.

This allows the kernel to receive input directly from the keyboard.

---

##🛠️ Technologies

- C — Kernel and system code
- Assembly — Boot and low-level code
- Make — Build system
- Linker Script — Kernel memory layout
- ISO — Bootable operating system image

---

##🎨 Visual Identity

- Aesthetic: Unix-like
- Style: Minimalist
- Design: Simple and clean

---

##🎯 Goals

- [ ] Custom kernel
- [x] C-based kernel
- [x] Boot process
- [x] Keyboard input
- [x] Text output
- [x] Bootable ISO
- [ ] Command system
- [ ] File system
- [ ] Memory management
- [ ] Hardware support
- [ ] Graphical interface

---

##🚧 Status

Something Project v0.2 is currently in development.

The project is focused on building the operating system from the kernel upward.

---

##📜 License

This project does not currently have a defined license.

:::
