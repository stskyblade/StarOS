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
    while (true) {
        ;
    }

    return 0;
}