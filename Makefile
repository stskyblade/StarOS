SRC=src
BUILD=build

CPPFLAG = -ffreestanding -Wall -Wno-write-strings -Wno-unused-variable -Wextra -fno-exceptions -fno-rtti -nostdlib -g -save-temps
QEMUOPT = -drive file=build/mbr.img,index=0,media=disk,format=raw,if=ide

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

# mbr.bin:
# 	#@cd $(SRC) && i686-elf-g++ -T mbr.ld -o ../$(BUILD)/mbr.elf $(CPPFLAG) mbr.S bootloader.S bootloader_32.cpp b32_print.cpp b32_disk.cpp
# # generate a symbol file for GDB, use another linker script, set offset at 0x00
# 	#@cd $(BUILD) && i686-elf-objcopy -O binary --only-section=.text mbr.elf mbr.bin
# # generate a file of 200MB
# 	#@cd $(BUILD) && dd status=none if=/dev/zero of=./mbr.img bs=512 count=409600
# # copy bin to disk img
# 	#@cd $(BUILD) && dd status=none if=./mbr.bin of=./mbr.img conv=notrunc
# # copy simple text file to disk img
# 	#@cd $(BUILD) && dd status=none if=../root_dir/hello.txt of=./mbr.img conv=notrunc seek=64
# 	#@cd $(BUILD) && dd status=none if=../root_dir/world.txt of=./mbr.img conv=notrunc seek=65

# bootloader.bin:
# 	@cd $(BUILD) && dd status=none if=/dev/zero of=./bootloader.bin bs=512 count=2
# 	@cd $(BUILD) && dd status=none if=../root_dir/hello.txt of=./bootloader.bin conv=notrunc seek=0
# 	@cd $(BUILD) && dd status=none if=../root_dir/world.txt of=./bootloader.bin conv=notrunc seek=1

# burn: mbr.bin
# 	@if [ ! -b /dev/sdb ]; then echo "File not found: /dev/sdb"; exit 1; fi
# # write 200KB to disk
# 	@sudo dd status=none if=build/disk.img of=/dev/sdb bs=512 count=512000
# 	@sync

dump:
	@sudo dd status=none if=/dev/sdb of=/tmp/sdb_250M.img bs=512 count=512000
	@sudo dd status=none if=/dev/sdb of=/tmp/sdb.img bs=512 count=400
	@hexdump -C /tmp/sdb.img
	@sudo dd status=none if=build/mbr.img of=/tmp/mbr_shorted.img bs=512 count=400
	diff /tmp/sdb.img /tmp/mbr_shorted.img -s

# qemu: mbr.bin
# 	qemu-system-i386 -serial stdio build/mbr.img

# debug: mbr.bin
# 	qemu-system-i386 build/mbr.img -s -S

gdb:
	gdb

fs:
	g++ src/tool/makefs.cpp -o makefs && ./makefs

clean:
	rm build/*
