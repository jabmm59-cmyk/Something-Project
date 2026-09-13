CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector
LDFLAGS = -m elf_i386

all: something-project-v0.2.iso

kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

print.o: print.c kernel.h keyboard.h
	$(CC) $(CFLAGS) -c print.c -o print.o

keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o

boot.o: boot.s
	$(CC) -m32 -c boot.s -o boot.o

kernel.bin: boot.o kernel.o print.o keyboard.o linker.ld
	$(LD) $(LDFLAGS) -T linker.ld \
	boot.o kernel.o print.o keyboard.o \
	-o kernel.bin

something-project-v0.2.iso: kernel.bin
	rm -rf iso
	mkdir -p iso/boot/grub

	cp kernel.bin iso/boot/kernel.bin

	printf 'menuentry "Something Project v0.2" {\n' \
		> iso/boot/grub/grub.cfg

	printf '    multiboot /boot/kernel.bin\n' \
		>> iso/boot/grub/grub.cfg

	printf '    boot\n' \
		>> iso/boot/grub/grub.cfg

	printf '}\n' \
		>> iso/boot/grub/grub.cfg

	grub-mkrescue -o something-project-v0.2.iso iso

clean:
	rm -rf *.o *.bin *.iso iso
