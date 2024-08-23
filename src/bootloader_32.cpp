// This is part of bootloader, but in 32-bit
// This code runs in 32bit protected mode, privilege 0, kernel mode(not in kernel)
// This code is loaded to memory address 0x7cxx, by bootloader.S

#include "bootloader32.h"

void print_data_type_size() {
    printf("Size of char 0x%x\n", sizeof(char));
    printf("Size of short 0x%x\n", sizeof(short));
    printf("Size of int 0x%x\n", sizeof(int));
    printf("Size of long 0x%x\n", sizeof(long));
    printf("Size of long long 0x%x\n", sizeof(long long));
}

extern "C" {

// never return
// entry
// jump from bootloader.S:jmp_32
void bootloader32_start() {
    // TODO: print hello,C. bootloader32
    // print_data_type_size();
    char *s = "hello,C. bootloader32.\n";
    printf(s);

    // uint8_t buffer[512];
    // read_disk_sector(8386, 1, buffer);
    // print_memory(buffer, 160);

    char *filename = "/hello.txt";
    uint8_t buffer1[512];
    fs_read(filename, buffer1);
    printf("%s:\n", filename);
    print_memory(buffer1, 32);

    filename = "/world.txt";
    uint8_t buffer2[512];
    fs_read(filename, buffer2);
    printf("%s:\n", filename);
    print_memory(buffer2, 32);

    filename = "/large_hello.txt";
    uint8_t buffer3[512 * 8];
    fs_read(filename, buffer3);
    printf("%s:\n", filename);
    print_memory(buffer3 + 1024, 160);

    while (1) {
        ;
    }
}
}
