#include "bootloader32.h"

int strcmp(const char *lhs, const char *rhs) {
    while (*lhs && *rhs) {
        if (*lhs == *rhs) {
            lhs++;
            rhs++;
            continue;
        } else if (*lhs < *rhs) {
            return -1;
        } else if (*lhs > *rhs) {
            return 1;
        }
    }

    if (*lhs == 0 && *rhs == 0) {
        return 0;
    } else if (*lhs == 0) {
        return -1;
    } else {
        return 1;
    }
}

void *memset(void *dest, uint8_t ch, uint64_t count) {
    uint8_t *buffer = (uint8_t *)dest;
    for (uint64_t i = 0; i < count; i++) {
        buffer[i] = ch;
    }
    return dest;
}

void *zeromem(void *dest, uint32_t count) {
    uint32_t four_byte_count = count / 4;
    uint32_t reminder = count % 4;

    uint32_t *buffer = (uint32_t *)dest;
    for (uint32_t i = 0; i < four_byte_count; i++) {
        buffer[i] = 0;
    }

    if (reminder) {
        uint8_t *rest_buffer = (uint8_t *)dest + four_byte_count * 4;
        for (uint32_t i = 0; i < reminder; i++) {
            rest_buffer[i] = 0;
        }
    }
    return dest;
}

void *memcpy(void *dest, const void *src, uint64_t count) {
    uint8_t *p1 = (uint8_t *)dest;
    uint8_t *p2 = (uint8_t *)src;
    while (count) {
        *p1 = *p2;
        p1++;
        p2++;
        count--;
    }
    return dest;
}