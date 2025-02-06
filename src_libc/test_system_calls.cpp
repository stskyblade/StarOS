#include "libc.h"

void syscall_test_helloworld() {
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"r"(SYSCALL_HELLOWORLD));
    __asm__ __volatile__("int $32\n\t" ::);
}

int syscall_test_add(int a, int b) {
    int result = 0;
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"i"(SYSCALL_ADD));

    __asm__ __volatile__("mov %0, %%ebx\n\t" ::"m"(a));
    __asm__ __volatile__("mov %0, %%ecx\n\t" ::"m"(b));
    __asm__ __volatile__("int $32\n\t" ::);
    __asm__ __volatile__("mov %%eax, %0\n\t" : "=rm"(result) :);
    return result;
}