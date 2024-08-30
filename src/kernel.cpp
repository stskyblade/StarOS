#include "bootloader32.h"

extern "C" {
void kernel_main() {
    printf("hello,kernel\n");

    // never return
    while (1) {
        ;
    }
}
}
