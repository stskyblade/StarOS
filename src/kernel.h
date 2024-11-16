#include <stdint.h>

// ====================== stdlib.cpp start ===========================

void *malloc(uint64_t size);
void *alloc_page();

// ====================== stdlib.cpp end ===========================

// ================== paging.cpp start ======================
bool ksetup_kernel_paging();
void add_paging_map(void *linear_address, void *physical_address);
// ================== paging.cpp end ======================

// ================== interrupt.cpp start ======================
void init_interrupt_handler();
// ================== interrupt.cpp end ======================

// ================== process.cpp start ======================
int execv(const char *pathname, char *const argv[]);
// ================== process.cpp end ======================
