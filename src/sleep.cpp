#include "kernel.h"

int64_t Count_down = 0; // -1 on each timer interrupt

void ksleep(uint32_t seconds) {
    Count_down = TICKS_PER_SECOND * seconds;

    while (Count_down >= 0) {
        __asm__ __volatile("hlt" ::);
    }
    return;
}