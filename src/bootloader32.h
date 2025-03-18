#pragma once
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

inline void io_wait(void) {
    outb(0x80, 0);
}

enum LoggingLevel {
    TRACE,
    DEBUG,
    INFO,
    ERROR,
    FATAL
};

const LoggingLevel CURRENT_LOGGING_LEVEL = INFO;
// b32_print
#define panic(...) \
    fatal(__VA_ARGS__)

#define trace(...)                        \
    if (CURRENT_LOGGING_LEVEL <= TRACE) { \
        printf("[TRACE]: ");              \
        printf(__VA_ARGS__);              \
        printf("\n");                     \
    }

#define debug(...)                        \
    if (CURRENT_LOGGING_LEVEL <= DEBUG) { \
        printf("[DEBUG]: ");              \
        printf(__VA_ARGS__);              \
        printf("\n");                     \
    }
#define info(...)                        \
    if (CURRENT_LOGGING_LEVEL <= INFO) { \
        printf("[INFO]: ");              \
        printf(__VA_ARGS__);             \
        printf("\n");                    \
    }
#define fatal(...)                        \
    if (CURRENT_LOGGING_LEVEL <= FATAL) { \
        printf("[FATAL]: ");              \
        printf(__VA_ARGS__);              \
        printf("\n");                     \
        while (1) {                       \
            ;                             \
        }                                 \
    }

void print_c(char c);
int printf(const char *restrict, ...);
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
    if (index == length) {
        // keep the only one zero. index == length == 30
        index--;
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
void *zeromem(void *dest, uint32_t count);
void *memset(void *dest, uint8_t ch, uint64_t count);
void *memcpy(void *dest, const void *src, uint64_t count);

struct ELF_HEADER {
    uint32_t magic_num;
    uint8_t bitness;    // 1 for 32bit, 2 for 64
    uint8_t endianness; // 1 for little endian, 2 for big endian
    uint8_t version;
    uint8_t ABI;
    uint8_t ABI_version;
    uint8_t reserved[7];
    uint16_t type; // object file type
    uint16_t machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;

} __attribute__((packed));

struct Program_header {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed));

struct Section_header {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} __attribute__((packed));

struct Memory_layout_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

// ====================== b32_utils.cpp start ===========================

void sleep(int seconds);

// ====================== b32_utils.cpp end ===========================
