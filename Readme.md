# StarOS
i386

## Build

```bash
git clone -b dev-process https://github.com/stskyblade/StarOS.git
cd StarOS
mkdir build
cd build
cmake ..
cmake --build . --target qemu
cmake --build . --target debug # for gdb debug
cd <Project_root>
gdbgui .  # start a gui gdb
```

## TODO
- [x] hello,world
- [x] write to serial port
- [x] read data from disk
- [ ] interrupt generate & handle & restore
- [ ] virtual memory, page table
- [ ] user mode, or protected mode
- [ ] clock interrupt
- [ ] schedular
- [ ] vga support newline, scoll down, colorful text

## Filesystem
- [x] generate disk image
- [x] open_dir
- [x] read_dir
- [x] fopen
- [x] fread

## Shell program in user mode
- [ ] system calls
- [ ] read/write standard input
- [ ] load Shell program into memory
- [ ] progress abstract, schedular
- [ ] allocate resources for Shell progress
- [ ] switch to Shell progress

## Priviledge mode for i386
difference, how to switch

Real mode:
when PC starts

Protected mode:
support virtual memory, should be used when run user space program

PE bit, CR0

## Page and segment memory management
PG bit, CR0

## IO
- [ ] interrupt register, meaning & manipulate
- [ ] special assemble command
- [ ] open interrupt & disable interrupt
- [ ] Interrupt Vector print error message
- [ ] restore old kernel program

### How to read input from serial?
- [x] 1, while loop, read sign register, wait for data
2, interrupt handler

## interrupt
- [x] disable/enable, IF in EFLAGS
- [x] find out what causes interrupt handler, is it time clock?
- [x] read interrupt message
- [x] restore
- [x] generate a software interrupt. INT instruction
- [x] handle interrupt, IDTR
- [x] time clock

write handler functions (256)
save the addresses of handler functions to LDT or GDT
save the trap gate in IDT
write the address of IDT to IDTR
enable interrupt
emit interrupt signal

### difference of IDT, GDT, LDT, which to use

## shortcut

press ctrl+a ctrl+a x, to exit qemu
ctrl+a, ctrl+a c, to see all registers

## how program start?

boot.s: _start

call kernel_main

kernel.cpp: kernel_main

set interrupt vector table in init_interrupt_handler in interrupt.cpp
