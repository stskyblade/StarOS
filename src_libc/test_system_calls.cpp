#include "libc.h"

void syscall_test_helloworld() {
    for (int i = 0; i < 3; i++) {
        __asm__ __volatile__("mov %0, %%eax\n\t" ::"r"(SYSCALL_HELLOWORLD));
        __asm__ __volatile__("int $32\n\t" ::);
    }
}