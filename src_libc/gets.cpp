#include "libc.h"

char *gets(char *buf, size_t buf_size) {
    char *result = 0;
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"i"(SYSCALL_GETS));
    __asm__ __volatile__("mov %0, %%ebx\n\t" ::"m"(buf));
    __asm__ __volatile__("mov %0, %%ecx\n\t" ::"m"(buf_size));
    __asm__ __volatile__("int $32\n\t" ::);
    __asm__ __volatile__("mov %%eax, %0\n\t" : "=m"(result) :);
    return result;
}