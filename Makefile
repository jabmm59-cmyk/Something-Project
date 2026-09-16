CC = gcc
LD = ld
NASM = nasm

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra \
         -fno-stack-protector -fno-pie -fno-pic

LDFLAGS = -m elf_i386


all: something-project-v0.3.iso


# ====================================
# BOOTLOADER
# ====================================

bootloader.bin: bootloader.asm
	$(NASM) -f bin bootloader.asm -o bootloader.bin


# ====================================
# KERNEL ENTRY
# ====================================

kernel_entry.o: kernel_entry.asm
	$(NASM) -f elf32 kernel_entry.asm -o kernel_entry.o


# ====================================
# KERNEL
# ====================================

kernel.o: kernel.c kernel.h commands.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o


print.o: print.c kernel.h keyboard.h
	$(CC) $(CFLAGS) -c print.c -o print.o


keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o


commands.o: commands.c commands.h kernel.h calculator.h sysinfo.h
	$(CC) $(CFLAGS) -c commands.c -o commands.o


calculator.o: calculator.c calculator.h kernel.h
	$(CC) $(CFLAGS) -c calculator.c -o calculator.o


sysinfo.o: sysinfo.c sysinfo.h kernel.h
	$(CC) $(CFLAGS) -c sysinfo.c -o sysinfo.o


# ====================================
# LINK DO KERNEL
# ====================================

kernel.bin: kernel_entry.o kernel.o print.o keyboard.o commands.o calculator.o sysinfo.o linker.ld
	$(LD) $(LDFLAGS) -T linker.ld \
		kernel_entry.o \
		kernel.o \
		print.o \
		keyboard.o \
		commands.o \
		calculator.o \
		sysinfo.o \
		-o kernel.bin \
		--oformat binary


# ====================================
# CRIAR IMAGEM DE DISCO
# ====================================

boot.img: bootloader.bin kernel.bin

	rm -f boot.img

	# 17 setores:
	# setor 0 = bootloader
	# setores 1-16 = kernel

	dd if=/dev/zero of=boot.img bs=512 count=17

	# Colocar bootloader no setor 0
	dd if=bootloader.bin of=boot.img conv=notrunc

	# Colocar kernel a partir do setor 1
	dd if=kernel.bin of=boot.img bs=512 seek=1 conv=notrunc


# ====================================
# ISO
# ====================================

something-project-v0.3.iso: boot.img

	rm -rf iso
	mkdir -p iso

	cp boot.img iso/boot.img

	xorriso -as mkisofs \
		-b boot.img \
		-no-emul-boot \
		-boot-load-size 17 \
		-o something-project-v0.3.iso \
		iso


# ====================================
# LIMPAR
# ====================================

clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.iso
	rm -f boot.img
	rm -rf iso
