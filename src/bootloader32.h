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
#define panic(...)       \
    printf(__VA_ARGS__); \
    while (1)            \
        ;
void print_c(char c);
void printf(const char *restrict, ...);
void print_memory(uint8_t *buf, int length);

// print a 16-bit unsigned `data` in hex representation
template <typename T>
void print_hex(T data) {
    // if (data == 0) {
    //     print_c('0');
    //     return;
    // }
    int length = sizeof(T) * 2; // hex representation length

    char *hex_str = "0123456789abcdef";
    char buffer[length];
    for (int i = 0; i < length; i++) {
        int reminder = data % 16;
        data = data / 16;
        buffer[length - 1 - i] = hex_str[reminder];
    }

    // remove zeros
    // int index = 0;
    // while (buffer[index] == '0') {
    //     index++;
    // 0
    for (int i = 0; i < length; i++) {
        print_c(buffer[i]);
    }
}

template <typename T>
void print_int(T data) {
    // if (data == 0) {
    //     print_c('0');
    //     return;
    // }
    int length = 30; // 64 bit integer is less than 30

    char *int_str = "0123456789";
    char buffer[length];
    for (int i = 0; i < length; i++) {
        int reminder = data % 10;
        data = data / 10;
        buffer[length - 1 - i] = int_str[reminder];
    }

    // remove zeros
    int index = 0;
    while (buffer[index] == '0') {
        index++;
    }
    for (int i = index; i < length; i++) {
        print_c(buffer[i]);
    }
}

// b32_disk.cpp
// for 12TB disk drive, need 2 * 10^10 sectors, larger than 32 bit unsigned int
// read `count` sectors, start at `sector_number`(count from 0), copy to memory at address `dest`
// return 0 on success
int read_disk_sector(uint64_t sector_number, uint64_t count, uint8_t *dest);

// b32_filesystem.cpp
// filesystem interfaces

// read file `filename` to `buffer`
// `filename` null-terminated string
// Return:
// 0 on success, others for error
int fs_read(const char *filename, uint8_t *buffer);

struct stat {
    uint32_t st_size;
};
// return  information  about  a file, in the buffer pointed to by statbuf
int stat(const char *pathname, struct stat *statbuf);

// -------------------- b32_filesystem.cpp end--------------------------

// b32_string.cpp

// Negative value if lhs appears before rhs in lexicographical order.
// Zero if lhs and rhs compare equal.
// Positive value if lhs appears after rhs in lexicographical order.
int strcmp(const char *lhs, const char *rhs);