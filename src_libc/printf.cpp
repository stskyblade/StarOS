#include "libc.h"

int printf(char *format, ...) {
    int pointer = (int)format;
    int size = strlen(format);
    __asm__ __volatile__("mov %0, %%ebx\n\t" ::"m"(pointer));
    __asm__ __volatile__("mov %0, %%ecx\n\t" ::"m"(size));

    // pass the stack pointer, which is the address of the last arg before ...
    // this line will modify EAX, so put EAX line below
    char *addr_of_last_arg = (char *)&format;
    __asm__ __volatile__("mov %0, %%edx\n\t" ::"m"(addr_of_last_arg));
    int size_of_last_arg = sizeof(format);
    __asm__ __volatile__("mov %0, %%esi\n\t" ::"m"(size_of_last_arg));
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"i"(SYSCALL_PRINTF));
    __asm__ __volatile__("int $32\n\t" ::);
    return size;
}