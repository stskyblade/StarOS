#include <stdint.h>

// Page 157. Figure 9-3
struct Gate_Descriptor {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t type : 5;
    uint8_t dpl : 2;
    uint8_t p : 1; // 1 for present, 0 for not-present exception
    uint16_t offset_high;
} __attribute__((packed));

// Page 94, Figure 5-3
struct SegmentDescriptor {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    //
    uint8_t type : 4;
    uint8_t s : 1; // 1 for application, 0 for system
    uint8_t dpl : 2;
    uint8_t p : 1;
    //
    uint8_t limit_high : 4;
    uint8_t avl : 1;
    uint8_t o : 1;
    uint8_t x : 1;
    uint8_t g : 1;
    //
    uint8_t base_high;
} __attribute__((packed));

// Page 132, TSS, Task State Segment
struct TSS {
    uint16_t back_line;
    uint16_t reserved1;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved2;
} __attribute__((packed));

// ====================== stdlib.cpp start ===========================

void *malloc(uint64_t size);
void *alloc_page();

// ====================== stdlib.cpp end ===========================

// ================== paging.cpp start ======================
struct PTE {
    uint8_t p : 1;
    uint8_t rw : 1;
    uint8_t user_or_supervisor : 1;
    uint8_t reserved1 : 2; // 00
    uint8_t access : 1;
    uint8_t dirty : 1;
    uint8_t reserved2 : 2; // 00
    uint8_t avail : 3;
    uint32_t address : 20;

} __attribute__((packed));

extern bool is_paging_enabled;
bool ksetup_kernel_paging();

// add normal memory mapping
void add_memory_mapping(void *linear_address, void *physical_address, PTE *&paging_directory);
// add kernel memory mapping
void add_paging_map(void *linear_address, void *physical_address);
// ================== paging.cpp end ======================

// ================== interrupt.cpp start ======================
void init_interrupt_handler();
// ================== interrupt.cpp end ======================

// ================== process.cpp start ======================
int execv(const char *pathname, char *const argv[]);
// ================== process.cpp end ======================
