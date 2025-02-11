#include "bootloader32.h"
#include "kernel.h"

const int table_length = 1024;
MemoryBlock Free_blocks[table_length];
int Blocks_count = 0;
bool malloc_initialized = false;

const int allocated_table_length = 1024 * 10;
MemoryBlock Allocated_blocks[allocated_table_length];
int Allocated_blocks_count = 0;

void _malloc_array_init() {
    // check the address of free blocks
    size_t p = (uint32_t)Free_blocks;
    if (p < data_memory_start || p > data_memory_end) {
        fatal("Wrong location of Free_blocks");
    }
    for (int i = 0; i < table_length; i++) {
        Free_blocks[i] = {0, 0, 0, false};
    }
    Free_blocks[Blocks_count] = {free_memory_start,
                                 free_memory_start + free_memory_length,
                                 free_memory_length, true};
    Blocks_count++;

    for (int i = 0; i < allocated_table_length; i++) {
        Allocated_blocks[i] = {0, 0, 0, false};
    }
}

// Array version: use an array to keep the info of free memory blocks
// align to 8 bytes
void *_malloc_array(size_t size) {
    if (!malloc_initialized) {
        _malloc_array_init();
        malloc_initialized = true;
    }

    // align to 8 bytes
    if (size % 8 != 0) {
        size += 8 - (size % 8);
    }

    // find memory block large enough
    for (int i = 0; i < Blocks_count; i++) {
        MemoryBlock &block = Free_blocks[i];
        if (!block.is_valid) {
            continue;
        }
        // first match
        if (block.size >= size) {
            void *p = (void *)block.start;
            block.start += size;
            block.size -= size;

            if (Allocated_blocks_count == allocated_table_length) {
                fatal("Allocated blocks array out of space");
            }
            Allocated_blocks[Allocated_blocks_count] = {(size_t)p, block.start,
                                                        size, true};
            Allocated_blocks_count++;
            return p;
        }
    }
    fatal("Out of free blocks");
    return 0;
}

// recycle memory
// not perfect, there could be many small piece memory after free
void _free_array(void *p) {
    // case 1: can merge
    // is_valid means I have found the allocated block
    MemoryBlock allocated{0, 0, 0, false};
    for (int i = 0; i < Allocated_blocks_count; i++) {
        MemoryBlock &allocated_block = Allocated_blocks[i];
        if (!allocated_block.is_valid) {
            // is_valid means the block has not been freed
            continue;
        }

        if (allocated_block.start == (size_t)p) {
            allocated = allocated_block;
            allocated.is_valid = true;
            allocated_block.is_valid = false;
            break;
        }
    }
    if (!allocated.is_valid) {
        fatal("Memory has been freed before");
    }

    for (int i = 0; i < Blocks_count; i++) {
        MemoryBlock &block = Free_blocks[i];
        if (!block.is_valid) {
            continue;
        }

        if (allocated.end == block.start) {
            // merge
            block.start = allocated.start;
            block.size += allocated.size;
            return;
        }
    }

    // case can't merge
    if (Blocks_count == table_length) {
        fatal("Free blocks array out of space");
    }
    Free_blocks[Blocks_count] = allocated;
    Blocks_count++;
}

// entry of memory management
// function startswith _ shouldn't be used directly
void *malloc(size_t size) {
    return _malloc_array(size);
}

void free(void *p) {
    return _free_array(p);
}