#include "libc.h"

int main() {
    __asm__ __volatile__("mov $0x123, %%eax\n\t" ::);
    __asm__ __volatile__("mov $0x456, %%ebx\n\t" ::);
    __asm__ __volatile__("mov $0x789, %%ecx\n\t" ::);
    __asm__ __volatile__("mov $0x357, %%edx\n\t" ::);
    for (int i = 0; i < 3; i++) {
        syscall_test_helloworld();
    }

    int a = 1;
    int b = 2;
    int c = syscall_test_add(a, b);

    b = 4;
    c = syscall_test_add(c, b);

    b = 16;
    c = syscall_test_add(c, b);

    TestPack p;
    p.a1 = 1;
    p.a2 = 2;
    p.a3 = 3;
    p.a4 = 4;
    p.a5 = 5;
    p.a6 = 6;
    p.a7 = 7;
    p.a8 = 8;
    p.a9 = 9;
    p.a10 = 10;
    syscall_test_struct_arg(p);

    printf("I'm in user mode.\n");
    printf("hello,world from user space %d %x %d\n", 16, 17, true);

    printf("Please enter a string, 29 chars(Press Enter key to finish): \n");
    const size_t buf_size = 5;
    char buf[buf_size];
    buf[buf_size - 1] = '\0';
    if (gets(buf, buf_size - 1)) {
        printf(buf);
    } else {
        printf("Error: gets failed");
    }

    printf("h");
    printf("h");
    printf("h");
    while (true) {
        printf("he");
        sleep(3);
    }

    return 0;
}