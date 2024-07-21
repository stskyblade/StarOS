SRC=src
BUILD=build

CPPFLAG = -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -nostdlib -g -save-temps
QEMUOPT = -drive file=rootfs.img,index=0,media=disk,format=raw,if=ide

RED=\033[0;32m
NC=\033[0m # No Color

source = boot.s kernel.cpp uart.cpp vga.cpp printf.cpp disk.cpp fs.cpp snippet.cpp interrupt.cpp vectors.S trapasm.S apic.cpp


kernel: vectors.S
	@cd $(SRC) && i686-elf-g++ -T linker.ld -o ../$(BUILD)/myos.bin $(CPPFLAG) $(source)
	@printf "${RED}Build kernel success!\n${NC}"

vectors.S: src/vectors.pl
	./src/vectors.pl > src/vectors.S

iso: kernel
	grub-file --is-x86-multiboot $(BUILD)/myos.bin && printf "${RED}Build kernel success.\n${NC}"
	mkdir -p isodir/boot/grub
	cp $(BUILD)/myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/myos.iso isodir

qemu: kernel
	qemu-system-i386 -kernel $(BUILD)/myos.bin -nographic $(QEMUOPT)

debug: kernel
	qemu-system-i386 -kernel $(BUILD)/myos.bin -s -S -nographic $(QEMUOPT)

gdb:
	gdb

fs:
	g++ src/tool/makefs.cpp -o makefs && ./makefs

clean:
	rm build/*
