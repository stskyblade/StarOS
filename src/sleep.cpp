#include "kernel.h"

int64_t Count_down = 0; // -1 on each timer interrupt

void usleep(uint32_t microseconds) {
    if (microseconds > 1000000) {
        fatal("microseconds too large: %d", microseconds);
    }
    Count_down = 1.0 * TICKS_PER_SECOND * microseconds / 1000000;

    while (Count_down > 0) {
        __asm__ __volatile("hlt" ::);
    }
    return;
}

void sleep(uint32_t seconds) {
    usleep(seconds * 1000000);
}