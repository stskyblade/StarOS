#include "system.h"

int main() {
    __asm__ __volatile__("mov $0x123, %%eax\n\t" ::);
    __asm__ __volatile__("mov $0x456, %%ebx\n\t" ::);
    __asm__ __volatile__("mov $0x789, %%ecx\n\t" ::);
    __asm__ __volatile__("mov $0x357, %%edx\n\t" ::);
    __asm__ __volatile__("int $32\n\t" ::);
    while (true) {
        ;
    }

    return 0;
}