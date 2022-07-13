#include "types.h"

int init_serial();
void write_serial(char a);

void terminal_initialize(void);
void terminal_putchar(char c);

void print(const char *c);
void printf(const char *fmt, ...);

bool read_sector(uint addr, size_t sector_count, uchar *buffer);

void list_dir(const char *dir);
dir_entry_t *read_dir(dir_t &d);

void memcopy(uchar *dest, uchar *source, size_t count);
bool strcmp(const char *a, const char *b);

static inline void outb(ushort port, uchar data) {
    asm volatile("out %0,%1"
                 :
                 : "a"(data), "d"(port));
}

static inline uchar inb(ushort port) {
    uchar data;

    asm volatile("in %1,%0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline ushort inw(ushort port) {
    ushort data;

    asm volatile("in %1,%0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}
