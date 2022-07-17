# StarOS
i386


## TODO
- [x] hello,world
- [x] write to serial port
- [x] read data from disk
- [ ] virtual memory, page table
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

## Page and segment memory management

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


## shortcut

press ctrl+a ctrl+a x, to exit qemu
