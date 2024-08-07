// This is part of bootloader, but in 32-bit
// This code runs in 32bit protected mode, privilege 0, kernel mode(not in kernel)
// This code is loaded to memory address 0x7cxx, by bootloader.S

#include "bootloader32.h"

extern "C" {

// never return
// entry
// jump from bootloader.S:jmp_32
void bootloader32_start() {
    // TODO: print hello,C. bootloader32
    char *s = "hello,C. bootloader32.\n";
    printf(s);
    char *s2 = "hello,C. bootloader32.\n";
    printf(s2);
    while (1) {
        ;
    }
}
}
