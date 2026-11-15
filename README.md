# Something Project

A small x86_64 operating system: boots with GRUB (Multiboot2), runs a shell, a file manager and a
nano-like text editor. Early development (v0.1.0).

## What works (and how it was verified)

| Feature | Verified how |
|---|---|
| In-RAM file system, path handling (`.`, `..`, absolute/relative) | 260 host tests, ASan + UBSan |
| Shell: `ls [-l]`, `cd`, `pwd`, `mkdir [-p]`, `rmdir`, `touch`, `rm [-r -f]`, `cat`, `echo`, `cp`, `mv`, `df`, `uname`, `clear`, `help`, `reboot`, `halt`, `>` / `>>` redirection, quotes | host tests |
| Keyboard decoding (US layout, shift, caps, ctrl, arrows, Del, Home/End, PgUp/PgDn) | host tests |
| Editor buffer logic (insert, delete, cursor, cut/paste, limits) | host tests |
| Kernel compiles and links freestanding, valid Multiboot2 header, 32-bit to 64-bit boot code | `make`, `make check`, disassembly |
| **Actually booting, screen output, editor and file-manager screens** | **NOT verified: no QEMU where this was written. Please run `make run`.** |

## Build and run

Needs: gcc, ld, python3, make; for the ISO: grub-mkrescue (+ xorriso, mtools); to run: qemu-system-x86_64.

    make            # build/kernel.elf
    make check      # validate the Multiboot2 header
    make test       # host-side tests
    make run        # build the ISO and start it in QEMU

If it does not boot, please report exactly what you see (blank screen, reboot loop, "EE" in red at the top-left
means GRUB did not pass a Multiboot2 handshake).

## Using it

    user@something:/home/user$ help
    nano notes.txt      editor:  ^O save  ^X exit  ^K cut line  ^U paste  ^G help
    fm                  file manager: arrows, Enter open, Bksp up, n new file, m new dir, r rename, Del delete, q quit

## Limits (by design, for now)

- The file system is in RAM; everything is lost on reboot. 255 entries, 192 data blocks, 4096 bytes per file.
- Keyboard layout is US (QEMU default). No ABNT2 yet. Keyboard is polled (no interrupts).
- No processes, no system calls, no user mode. Everything runs in the kernel.
- Editor: lines longer than 80 columns are cut off on screen (the data is kept).
- Text mode only (VGA 80x25). Boot is BIOS; UEFI-only firmware will not show text mode.

## Not implemented (needs the pieces above first)

Compilers and runtimes for Nativa, C/C++, Python, Lua, JavaScript, Rust, Java, HTML/CSS; a disk file system;
a graphical desktop. These require processes, system calls, a C library and storage, in that order.
