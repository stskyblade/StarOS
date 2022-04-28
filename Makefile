SRC=src
BUILD=build

RED=\033[0;32m
NC=\033[0m # No Color

kernel:
	cd $(SRC) && i686-elf-as boot.s -o boot.o -g
	cd $(SRC) && i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
	cd $(SRC) && i686-elf-gcc -T linker.ld -o ../$(BUILD)/myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc -g
	grub-file --is-x86-multiboot $(BUILD)/myos.bin && printf "${RED}Build kernel success.\n${NC}"
	mkdir -p isodir/boot/grub
	cp $(BUILD)/myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/myos.iso isodir

qemu: kernel
	qemu-system-i386 -kernel $(BUILD)/myos.bin

debug: kernel
	qemu-system-i386 -kernel $(BUILD)/myos.bin -s -S

gdb:
	gdb
