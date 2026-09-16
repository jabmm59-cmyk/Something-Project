# 🚀 Something Project

> **A small experimental operating system written mainly in C.**

---

## 📌 About the Project

**Something Project** is an experimental operating system and kernel project created to explore how an operating system works from the ground up.

The main goal is to make the kernel programming experience as simple and familiar as possible, using **normal C syntax** while hiding low-level hardware operations behind simple functions.


The project is currently in **early development**.

---

## 🏷️ Current Version

**Something Project v0.2.1**

### What's new in v0.2.1?

- ⌨️ Keyboard input
- 📝 Text input
- 🔢 Integer input and output
- 🧮 Basic calculator
- 🔄 Infinite command loop
- 🥾 Custom bootloader
- 💻 32-bit kernel
- 🧩 Simple kernel API
- 💾 Small bootable ISO

---

## ✨ Features

| Feature | Status |
|---|---|
| C kernel | ✅ |
| 32-bit support | ✅ |
| Text output | ✅ |
| Keyboard input | ✅ |
| Integer input | ✅ |
| Integer output | ✅ |
| Calculator | ✅ |
| Command menu | ✅ |
| Custom bootloader | ✅ |
| File system | 🚧 |
| Memory management | 🚧 |
| Multitasking | ❌ |
| User programs | ❌ |
| Networking | ❌ |
| GUI | ❌ |

> **Legend:**  
> ✅ Implemented  
> 🚧 In development  
> ❌ Not implemented

---

## 🖥️ Example

When Something Project starts, it displays a simple command interface:

```text
Something Project v0.2
live@user S~/:

1 - Calculator
Choose:
```

The calculator supports:

```text
+
-
*
/
```

---

## 🧠 Philosophy

Something Project is designed around a simple idea:

> **Make low-level operating system development feel more like normal C programming.**

Instead of writing hardware-specific code every time something needs to be displayed, the project provides simple functions such as:

```c
print();
input();
input_int();
print_int();
```

This allows higher-level kernel code to remain simple.

---

## 🏗️ Architecture

The current boot process is approximately:

```text
BIOS
 │
 ▼
Custom Bootloader
 │
 ▼
32-bit Protected Mode
 │
 ▼
Kernel Entry
 │
 ▼
kernel_main()
 │
 ├── print()
 ├── input()
 ├── input_int()
 └── print_int()
```

---

## 📁 Project Structure

```text
Something Project v0.2/
│
├── bootloader.asm
├── kernel_entry.asm
│
├── kernel.c
├── kernel.h
│
├── print.c
│
├── keyboard.c
├── keyboard.h
│
├── linker.ld
└── Makefile
```

### 📄 File descriptions

- **`bootloader.asm`** — Custom bootloader.
- **`kernel_entry.asm`** — Entry point of the kernel.
- **`kernel.c`** — Main kernel code.
- **`kernel.h`** — Kernel function declarations.
- **`print.c`** — Text and integer input/output functions.
- **`keyboard.c`** — Keyboard handling.
- **`keyboard.h`** — Keyboard function declarations.
- **`linker.ld`** — Kernel memory layout.
- **`Makefile`** — Automatic build system.

---

## 🛠️ Building

### Requirements

On **Arch Linux**, install the required tools:

```bash
sudo pacman -S gcc nasm xorriso
```

Check the installed versions:

```bash
gcc --version
nasm -version
xorriso --version
```

---

## 🔨 Compile

Clone the repository:

```bash
git clone https://github.com/jabmm59-cmyk/Something-Project.git
```

Enter the project directory:

```bash
cd Something-Project
```

Build the project:

```bash
make
```

Clean the generated files:

```bash
make clean
```

After a successful build, the ISO should be generated as:

```text
something-project-v0.2.iso
```

If you don't want to do the ISO in the terminal go to the tags of this repository and go to the Tag:

```text
Something Project ISO v0.2.1
```

---

## 🧪 Testing

The ISO can be tested using **QEMU**:

```bash
qemu-system-i386 -cdrom something-project-v0.2.iso
```

You can also test it using other virtual machines or compatible hardware.

> ⚠️ **Warning:** Something Project is experimental software.  
> It may not work correctly on all hardware or virtual machines.

---

## 🗺️ Roadmap

### Kernel

- [x] Basic kernel
- [x] Text output
- [x] Keyboard input
- [x] Integer input
- [x] Integer output
- [x] Basic command system
- [x] Calculator
- [ ] Better keyboard driver
- [ ] Improved input system
- [ ] Memory management
- [ ] Dynamic memory allocation
- [ ] Interrupt handling
- [ ] System calls
- [ ] Multitasking

### Storage

- [ ] Disk driver
- [ ] File system
- [ ] File reading
- [ ] File writing
- [ ] File management commands

### User Environment

- [ ] Shell
- [ ] User programs
- [ ] Program loading
- [ ] More commands
- [ ] Configuration system

### Graphics

- [ ] Graphics mode
- [ ] Framebuffer
- [ ] Basic graphics library
- [ ] Window system
- [ ] GUI

---

## 💻 Supported Architecture

Currently:

```text
Architecture: x86
Mode:         32-bit
Kernel:       Custom
Language:     C + Assembly
Bootloader:   Custom
```

---

## 📦 ISO

The project generates a small bootable ISO.

The goal is to keep the system lightweight instead of including a large external bootloader such as GRUB.

The source code and build system are included in this repository.

---

## 📚 Learning Goals

Something Project is also a learning project.

The project is being developed to understand concepts such as:

- **Operating system kernels**
- **Bootloaders**
- **CPU modes**
- **Memory**
- **Hardware communication**
- **Keyboard drivers**
- **Input/output**
- **Compilers**
- **Linkers**
- **Assembly**
- **C programming**

---

## 🤝 Contributing

Contributions, ideas and bug reports are welcome!

If you find a bug or have an idea:

1. Open an **Issue**.
2. Explain the problem or idea.
3. If possible, provide steps to reproduce the problem.
4. Pull requests are welcome.

---

## 📜 License

This project is licensed under the **MIT License**.

See the [`LICENSE`](LICENSE) file for the full license text.

---

## 👨‍💻 Author

Created by **Jabmm59**.

---

## ⭐ Something Project

> **Small kernel. Big experiments.**

**Something Project v0.2** 

