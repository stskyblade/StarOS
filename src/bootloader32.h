#include <stdint.h>

inline uint8_t inb(uint16_t port) {
    uint8_t data;

    __asm__ __volatile__("inb %1, %0\n\t"
                         : "=a"(data)
                         : "d"(port));
    return data;
}

inline uint16_t inw(uint16_t port) {
    uint16_t data;

    __asm__ __volatile__("inw %1, %0\n\t"
                         : "=a"(data)
                         : "d"(port));
    return data;
}

inline uint32_t inl(uint16_t port) {
    uint32_t data;

    __asm__ __volatile__("inl %1, %0\n\t"
                         : "=a"(data)
                         : "d"(port));
    return data;
}

inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %0, %1\n\t"
                         :
                         : "a"(data), "d"(port));
}

inline void outl(uint16_t port, uint32_t data) {
    __asm__ __volatile__("outl %0, %1\n\t"
                         :
                         : "a"(data), "d"(port));
}

// b32_print
void printf(const char *restrict, ...);

// b32_disk.cpp
// for 12TB disk drive, need 2 * 10^10 sectors, larger than 32 bit unsigned int
// read `count` sectors, start at `sector_number`(count from 0), copy to memory at address `dest`
// return 0 on success
int read_disk_sector(uint64_t sector_number, uint64_t count, uint8_t *dest);