#include "system.h"

int main() {
    __asm__ __volatile__("int $32\n\t" ::);
    while (true) {
        ;
    }

    return 0;
}