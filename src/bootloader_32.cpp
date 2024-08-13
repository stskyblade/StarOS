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
    char *s2 = "hello,C. bootloader32.\n";
    printf(s2);

    uint8_t buffer[1024];
    read_disk_sector(0, 1, buffer);

    while (1) {
        ;
    }
}
}
