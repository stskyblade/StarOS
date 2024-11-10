#include <stdint.h>

// ====================== stdlib.cpp start ===========================

void *malloc(uint64_t size);
void *alloc_page();

// ====================== stdlib.cpp end ===========================

// ================== paging.cpp start ======================
bool ksetup_kernel_paging();
// ================== paging.cpp end ======================