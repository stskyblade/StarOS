#include "bootloader32.h"

void sleep(int seconds) {
    // it takes about 10 seconds for UINT32_MAX in laptop, 20 seconds for Qemu
    for (uint32_t i = 0; i < UINT32_MAX / 10 * seconds; i++) {
        /* code */
    }
}