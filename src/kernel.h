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

  public:
    SegmentDescriptor()
        : SegmentDescriptor(0, 0, 0, 0) {
    }

    SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags) {
        limit_low = limit & 0xffff;     // low 16 bit
        base_low = base & 0xffff;       // low 16 bit
        base_mid = (base >> 16) & 0xff; // mid 8 bit

        type = access_byte & 0xf;        // low 4 bit
        s = (access_byte >> 4) & 0b1;    // low 1 bit
        dpl = (access_byte >> 5) & 0b11; // low 2 bit
        p = (access_byte >> 7) & 0b1;    // low 1 bit

        limit_high = (limit >> 16) & 0xf; // low 4 bit
        avl = flags & 0b1;
        o = (flags >> 1) & 0b1;
        x = (flags >> 2) & 0b1;
        g = (flags >> 3) & 0b1;

        base_high = (base >> 24);
    }
} __attribute__((packed));

struct GDTR {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Page 132, TSS, Task State Segment
struct TSS {
    uint16_t back_link;
    uint16_t reserved; // zeros
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved0; // zeros
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved1; // zeros
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved2; // zeros

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t reserved3; // zeros
    uint16_t cs;
    uint16_t reserved4; // zeros
    uint16_t ss;
    uint16_t reserved5; // zeros
    uint16_t ds;
    uint16_t reserved6; // zeros
    uint16_t fs;
    uint16_t reserved7; // zeros
    uint16_t gs;
    uint16_t reserved8; // zeros
    uint16_t ldt;
    uint16_t reserved9; // zeros

    uint16_t t : 1;
    uint16_t reserved10 : 15;
    uint16_t io_map_base;
} __attribute__((packed));

extern TSS *KERNEL_TSS;

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
