#include "bootloader32.h"

extern "C" {
void kernel_main() {
    printf("hello,kernel\n");
    return;
}
}
