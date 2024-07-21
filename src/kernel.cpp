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

void f(){};
int a[10];

void test_data_type_size() {
    printf("Size of short: %d\n", sizeof(short));
    printf("Size of int: %d\n", sizeof(int));
    printf("Size of long: %d\n", sizeof(long));
    printf("Size of pointer of int: %d\n", sizeof(int *));
    printf("Size of pointer of long: %d\n", sizeof(long *));
    printf("Size of pointer of function: %d\n", sizeof(&f));
    printf("Size of array of int: %d\n", sizeof(&a));
    /*
      Size of short: 2
      Size of int: 4
      Size of long: 4
      Size of pointer of int: 4
      Size of pointer of long: 4
      Size of pointer of function: 4
      Size of array of int: 4
     */
}

extern "C" { // use c linkage to avoiding name mangling
void kernel_main(void) {
    // print("\nHello, World!\n");

    // // for (int i = 0; i < 256; i++) {
    // //     uchar c = i;
    // //     printf("0x%x %x \n", c, c + 1);
    // // }

    // // uchar buffer[512 * 10];
    // // read_sector(1, 1, buffer);

    // list_dir("/");
    // FILE *f = fopen("/world.txt");
    // char str[2048];
    // fread(str, 1024, 1, f);
    // str[1024] = '\0';
    // printf("\nContent of file /world.txt:\n");
    // printf(str);

    // test_data_type_size();

    // printf("\nTest read from terminal:\n");
    // while (true) {
    //     test_read_serial();
    //     printf("\n");
    // }

    // enter protected mode
    // mov eax, CR0
    // or eax, 1
    // mov CR0, eax
    asm volatile("mov %CR0,%eax");
    asm volatile("or $1,%eax");
    asm volatile("mov %eax,%CR0");
    // TODO: need a far jmp or far call here

    printf("\nIn protected mode\n");

    // init_interrupt_handler();
    // init_apic(); // timer related

    printf("\nkernel main looping...\n");
    while (true) {
        ;
    }
}
}
