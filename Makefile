SRC=src
BUILD=build

RED=\033[0;32m
NC=\033[0m # No Color

obj= boot.o kernel.o uart.o

kernel:
	@cd $(SRC) && i686-elf-as boot.s -o boot.o -g
	@cd $(SRC) && i686-elf-g++ -c kernel.cpp -o kernel.o -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -g
	@cd $(SRC) && i686-elf-g++ -c uart.cpp -o uart.o -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -g
	@cd $(SRC) && i686-elf-gcc -T linker.ld -o ../$(BUILD)/myos.bin -ffreestanding -nostdlib -lgcc -g $(obj)
	@printf "${RED}Build kernel success!\n${NC}"

iso: kernel
	grub-file --is-x86-multiboot $(BUILD)/myos.bin && printf "${RED}Build kernel success.\n${NC}"
	mkdir -p isodir/boot/grub
	cp $(BUILD)/myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/myos.iso isodir

qemu: kernel
	qemu-system-i386 -kernel $(BUILD)/myos.bin -nographic

debug: kernel
	qemu-system-i386 -kernel $(BUILD)/myos.bin -s -S -nographic

gdb:
	gdb


clean:
	rm src/*.o
	rm build/*
