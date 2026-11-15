#!/usr/bin/env python3
"""Sanity-check that an ELF64 kernel has a valid Multiboot2 header. Does not boot anything."""
import struct
import sys

MAGIC = 0xE85250D6


def main(path):
    data = open(path, "rb").read()
    if data[:4] != b"\x7fELF" or data[4] != 2:
        return fail("not an ELF64 file")
    machine, = struct.unpack_from("<H", data, 18)
    entry, = struct.unpack_from("<Q", data, 24)
    if machine != 0x3E:
        return fail("not x86_64")
    if entry >= 2**32:
        return fail(f"entry point {entry:#x} does not fit in 32 bits (GRUB enters in 32-bit mode)")
    window = data[:32768]
    for off in range(0, len(window) - 16, 8):
        magic, arch, length, checksum = struct.unpack_from("<IIII", window, off)
        if magic != MAGIC:
            continue
        if (magic + arch + length + checksum) & 0xFFFFFFFF != 0:
            return fail(f"bad checksum at offset {off:#x}")
        end_type, end_flags, end_size = struct.unpack_from("<HHI", window, off + length - 8)
        if (end_type, end_size) != (0, 8):
            return fail("missing end tag")
        print(f"OK: Multiboot2 header at file offset {off:#x}, length {length}, entry {entry:#x}")
        return 0
    return fail("no Multiboot2 header in the first 32 KiB")


def fail(msg):
    print("FAIL:", msg)
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
