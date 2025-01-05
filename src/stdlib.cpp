#include "bootloader32.h"
#include "kernel.h"

uint8_t *free_memory = reinterpret_cast<uint8_t *>(free_memory_start);
uint8_t *free_memory_page = (uint8_t *)(free_memory_page_start);

// Return a block of memory at least `size` bytes, aligned to 8-byte boundary
// Physical memory address
void *malloc(int64_t size) {
    void *allocated_memory = free_memory;
    free_memory += size;

    // align to 8-byte
    while ((uint64_t)free_memory % 8) {
        free_memory = free_memory + 1;
    }

    if (is_paging_enabled) {
        // add kernel paging map if necessary
        uint32_t addr = (uint32_t)allocated_memory & (~0xFFF);
        add_kernel_memory_mapping((void *)addr, (void *)addr);

        // add more mappings if size is larger than 4KB
        while (size > PAGE_SIZE) {
            addr += PAGE_SIZE;
            add_kernel_memory_mapping((void *)addr, (void *)addr);
            size -= PAGE_SIZE;
        }
    }
    return allocated_memory;
}

// Return a 4KB memory block, aligned at 4KB boundary
void *alloc_page() {
    // info("Free memory: 0x%x, 0x%x", (uint32_t)free_memory,
    // (uint32_t)free_memory_start);
    while ((uint32_t)free_memory_page % PAGE_SIZE) {
        free_memory_page = free_memory_page + 1;
    }
    void *allocated_memory = free_memory_page;
    free_memory_page += PAGE_SIZE;

    // if (is_paging_enabled) {
    //     // add kernel paging map if necessary
    //     uint32_t addr = (uint32_t)allocated_memory & (~0x111);
    //     add_paging_map((void *)addr, (void *)addr);
    // }

    // memset(allocated_memory, 0, size);

    return allocated_memory;
}
