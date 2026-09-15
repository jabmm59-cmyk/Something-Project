CC = gcc
LD = ld
NASM = nasm

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-pie -fno-pic
LDFLAGS = -m elf_i386

all: something-project-v0.2.iso

bootloader.bin: bootloader.asm
	$(NASM) -f bin bootloader.asm -o bootloader.bin

kernel_entry.o: kernel_entry.asm
	$(NASM) -f elf32 kernel_entry.asm -o kernel_entry.o

kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

print.o: print.c kernel.h keyboard.h
	$(CC) $(CFLAGS) -c print.c -o print.o

keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o

kernel.bin: kernel_entry.o kernel.o print.o keyboard.o linker.ld
	$(LD) $(LDFLAGS) -T linker.ld \
		kernel_entry.o \
		kernel.o \
		print.o \
		keyboard.o \
		-o kernel.bin \
		--oformat binary

something-project-v0.2.iso: bootloader.bin kernel.bin
	rm -rf iso
	mkdir -p iso

	dd if=/dev/zero of=boot.img bs=512 count=128
	dd if=bootloader.bin of=boot.img conv=notrunc
	dd if=kernel.bin of=boot.img bs=512 seek=1 conv=notrunc

	xorriso -as mkisofs \
		-b boot.img \
		-no-emul-boot \
		-boot-load-size 128 \
		-o something-project-v0.2.iso \
		iso

clean:
	rm -f *.o *.bin *.iso boot.img
	rm -rf iso
