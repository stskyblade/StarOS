#include "kernel.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error \
    "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

extern "C" { // use c linkage to avoiding name mangling
void kernel_main(void) {
    print("\nHello, World!\n");

    // for (int i = 0; i < 256; i++) {
    //     uchar c = i;
    //     printf("0x%x %x \n", c, c + 1);
    // }

    // uchar buffer[512 * 10];
    // read_sector(1, 1, buffer);

    list_dir("/");
    FILE *f = fopen("/world.txt");
    char str[2048];
    fread(str, 1024, 1, f);
    str[1024] = '\0';
    printf("\nContent of file /world.txt:\n");
    printf(str);
}
}
