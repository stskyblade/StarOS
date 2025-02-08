#include "libc.h"

int printf(char *format) {
    int pointer = (int)format;
    int size = strlen(format);
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"i"(SYSCALL_PRINTF));
    __asm__ __volatile__("mov %0, %%ebx\n\t" ::"m"(pointer));
    __asm__ __volatile__("mov %0, %%ecx\n\t" ::"m"(size));
    __asm__ __volatile__("int $32\n\t" ::);
    return size;
}