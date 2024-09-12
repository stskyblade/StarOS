// ## 总结：Qemu 和笔记本都能用的内存区域

// 取二者重合的话
// 0x100000 - 0x20000000, 大概 500MB.   这部分用于内核的代码和数据
// 0x20200000 - 0x40004000，大概 500MB  自由分配
// 0x40005000 - 0xbffe0000，大概 2GB    自由分配

#include "kernel.h"

void *free_memory = (void *)0x20200000;

// Return a block of memory at least `size` bytes, aligned to 8-byte boundary
void *malloc(uint64_t size) {
    void *allocated_memory = free_memory;
    free_memory += size;
    while ((uint64_t)free_memory % 8) {
        free_memory = free_memory + 1;
    }
    return allocated_memory;
}