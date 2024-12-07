// ## 总结：Qemu 和笔记本都能用的内存区域

// 取二者重合的话
// 0x100000 - 0x20000000, 大概 500MB.   这部分用于内核的代码和数据
// 0x20200000 - 0x40004000，大概 500MB  自由分配
// 0x40005000 - 0xbffe0000，大概 2GB    自由分配

#include "bootloader32.h"
#include "kernel.h"

uint8_t *free_memory = reinterpret_cast<uint8_t *>(0x20200000);

// Return a block of memory at least `size` bytes, aligned to 8-byte boundary
// Physical memory address
void *malloc(uint64_t size) {
    void *allocated_memory = free_memory;
    free_memory += size;

    // align to 8-byte
    while ((uint64_t)free_memory % 8) {
        free_memory = free_memory + 1;
    }

    if (is_paging_enabled) {
        // add kernel paging map if necessary
        uint32_t addr = (uint32_t)allocated_memory & (~0xFFF);
        add_paging_map((void *)addr, (void *)addr);
    }
    return allocated_memory;
}

// Return a 4KB memory block, aligned at 4KB boundary
void *alloc_page() {
    int size = 1024 * 4;
    while ((uint32_t)free_memory % size) {
        free_memory = free_memory + 1;
    }
    void *allocated_memory = free_memory;
    free_memory += size;

    // if (is_paging_enabled) {
    //     // add kernel paging map if necessary
    //     uint32_t addr = (uint32_t)allocated_memory & (~0x111);
    //     add_paging_map((void *)addr, (void *)addr);
    // }

    // memset(allocated_memory, 0, size);

    return allocated_memory;
}
