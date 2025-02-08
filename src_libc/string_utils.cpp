#include "libc.h"

uint32_t strlen(const char *str) {
    uint32_t count = 0;
    while (*str != '\0') {
        str++;
        count++;
    }
    return count;
}