CC ?= gcc
LD ?= ld
PYTHON ?= python3
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -DKERNEL -ffreestanding -fno-stack-protector -fno-pic -fno-pie \
         -mno-red-zone -mgeneral-regs-only -fno-tree-loop-distribute-patterns \
         -fno-asynchronous-unwind-tables -fcf-protection=none -m64 -Isrc
LDFLAGS = -n -nostdlib -T linker.ld -z max-page-size=0x1000 --build-id=none --no-warn-rwx-segments

KSRC = kmain console keyboard keymap fs shell editor_core editor fm ui mem
KOBJ = $(addprefix build/,$(addsuffix .o,$(KSRC))) build/boot.o

.PHONY: all test check iso run clean

all: build/kernel.elf

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/boot.o: src/boot.S
	@mkdir -p build
	$(CC) -m64 -c $< -o $@

build/kernel.elf: $(KOBJ) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KOBJ)

check: build/kernel.elf
	$(PYTHON) tools/check_multiboot.py build/kernel.elf

test:
	@mkdir -p build
	$(CC) -std=gnu11 -g -Wall -Wextra -fsanitize=address,undefined -Isrc -o build/test_all \
		tests/test_all.c tests/stubs.c src/fs.c src/shell.c src/keymap.c src/editor_core.c
	./build/test_all

iso: build/kernel.elf check
	@mkdir -p build/iso/boot/grub
	cp build/kernel.elf build/iso/boot/kernel.elf
	cp grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o build/somethingos.iso build/iso

run: iso
	qemu-system-x86_64 -cdrom build/somethingos.iso -m 256M

clean:
	rm -rf build
