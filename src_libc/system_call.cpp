// This file is the interface which is used by user level program to access
// system calls.
// This is part of libc.
inline void system_call(int id) {
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"m"(id));
    __asm__ __volatile__("int $32\n\t" ::);
}
