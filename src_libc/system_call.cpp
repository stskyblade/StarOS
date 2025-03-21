#include "libc.h"
// This file is the interface which is used by user level program to access
// system calls.
// This is part of libc.
inline void system_call(int id) {
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"m"(id));
    __asm__ __volatile__("int $32\n\t" ::);
}

void sleep(uint32_t seconds) {
    char *result = 0;
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"i"(SYSCALL_SLEEP));
    __asm__ __volatile__("mov %0, %%ebx\n\t" ::"m"(seconds));
    __asm__ __volatile__("int $32\n\t" ::);
    __asm__ __volatile__("mov %%eax, %0\n\t" : "=m"(result) :);
    return;
}