#include "bootloader32.h"
#include "kernel.h"

PTE *kernel_paging_directory = nullptr;
bool is_paging_enabled = false;

// add mapping to kernel space
void add_kernel_memory_mapping(void *linear_address, void *physical_address) {
    return add_memory_mapping(linear_address, physical_address,
                              kernel_paging_directory, false);
}

// add mapping to given page table
void check_address_mapping(void *addr, const PTE *paging_directory) {
    uint32_t address = (uint32_t)addr;
    debug("check mapping: 0x%x...", address);
    debug("page_directory: 0x%x", (uint32_t)paging_directory);
    const PTE &dir_entry = paging_directory[address >> 22];
    int dir_index = address >> 22;
    int dir_offset = dir_index * sizeof(PTE);
    debug("dir_index: 0x%x,  dir_offset: 0x%x, entry.p: %d", dir_index,
          dir_offset, dir_entry.p);
    debug("dir_entry address: 0x%x", (uint32_t)&dir_entry);
    if (!dir_entry.p) {
        info("address 0x%x not mapped in paging_directory 0x%x level 1",
             address, (uint32_t)paging_directory);
        return;
    }

    PTE *page_table = reinterpret_cast<PTE *>(
        reinterpret_cast<uint32_t>(dir_entry.address) << 12);
    PTE &entry = page_table[reinterpret_cast<uint32_t>(address) >> 12 &
                            0b1111111111]; // middle 10bit
    debug("entry.address: 0x%x", dir_entry.address);
    debug("page_table: 0x%x", (int)page_table);
    int table_index = reinterpret_cast<uint32_t>(address) >> 12 & 0b1111111111;
    int table_offset = table_index * sizeof(PTE);
    debug("table_index: 0x%x, table_offset: 0x%x, entry.p: %d", table_index,
          table_offset, entry.p);
    debug("table entry address: 0x%x", (int)&entry);

    if (!entry.p) {
        info("address 0x%x not mapped in paging_directory 0x%x level 2",
             address, (uint32_t)paging_directory);
        return;
    }
    uint32_t physical_addr = (entry.address << 12) + (address & 0xfff);
    debug("check mapping: 0x%x -> 0x%x success.", address, physical_addr);
}

// add a Page Table Entry
// map `linear_address` to `physical_address`, 4KB
// FIXME:linear_address provide DIR index & PTE index. So page table is not
// added to directory at index 0, 1, 2... but index comes from linear_address.
// 1024 entries match 10-bit dir, 10-bit page
void add_memory_mapping(void *linear_address, void *physical_address,
                        PTE *&paging_directory, bool user_level) {
    // debug("page mapping 0x%x -> 0x%x", (uint32_t)linear_address,
    //      (uint32_t)physical_address);
    trace("step1");
    if (sizeof(PTE) != 4) {
        panic("expected PTE size to be 4");
    }

    int linear = (int)linear_address;
    int physical = (int)physical_address;
    // printf("map %x -> %x\n", linear, physical);
    // check directory is valid
    // check directory[dir] is valid
    // check page_table[page] is valid

    if (reinterpret_cast<uint32_t>(linear_address) & 0b111111111111 ||
        reinterpret_cast<uint32_t>(physical_address) & 0b111111111111) {
        // linear_address is not aligned to 4KB boundary
        panic("address is not aligned to 4KB boundary");
    }

    trace("step2");
    PTE &dir_entry =
        paging_directory[reinterpret_cast<uint32_t>(linear_address) >> 22];
    int dir_entry_offset = reinterpret_cast<uint32_t>(linear_address) >> 22;
    if (!dir_entry.p) { // invalid dir_entry
        // allocate a new page table, and set up dir_entry
        trace("step2b");
        PTE *page_table = reinterpret_cast<PTE *>(alloc_page());

        dir_entry.p = 1;
        dir_entry.rw = 1;
        dir_entry.user_or_supervisor = user_level;
        dir_entry.reserved1 = 0;
        dir_entry.access = 0;
        dir_entry.dirty = 0;
        dir_entry.reserved2 = 0;
        dir_entry.avail = 0;
        dir_entry.address =
            reinterpret_cast<uint32_t>(page_table) >> 12; // save high 20bit
    }

    trace("step3");
    // use dir_entry.address, page part in linear_address to calculate the
    // location of page table entry
    PTE *page_table = reinterpret_cast<PTE *>(
        reinterpret_cast<uint32_t>(dir_entry.address) << 12);
    int entry_offset = reinterpret_cast<uint32_t>(linear_address) >> 12 &
                       0b1111111111; // middle 10bit
    trace("step3.1 page table at 0x%x", (uint32_t)page_table);
    if ((uint32_t)page_table < 0x78405000 ||
        (uint32_t)page_table > 0x80000000) {
        // sleep(1);
    }

    PTE &entry = page_table[entry_offset];
    trace("step3.2 entry at 0x%x", (uint32_t)&entry);
    entry.p = 1;
    entry.rw = 1;
    trace("step3.3");
    entry.user_or_supervisor = user_level;
    trace("step3.4");
    entry.reserved1 = 0;
    trace("step3.4.1");
    entry.access = 0;
    trace("step3.4.2");
    entry.dirty = 0;
    trace("step3.4.3");
    entry.reserved2 = 0;
    trace("step3.4.4");
    entry.avail = 0;
    trace("step3.5");
    entry.address =
        reinterpret_cast<uint32_t>(physical_address) >> 12; // save high 20bit
    trace("step4");
}

// add kernel mappings into kernel paging directory
void prepare_kernel_paging() {
    kernel_paging_directory = (PTE *)alloc_page();
    add_kernel_mappings(kernel_paging_directory);
}

// add kernel mappings into target paging directory
// set up page tables, linear memory address map to same physical memory address
void add_kernel_mappings(PTE *&page_directory) {
    for (int index = 0; index < kernel_maps_length; index++) {
        trace("index: %d/%d", index, kernel_maps_length);
        MemoryMap map = Kernel_maps[index];
        uint32_t pages_count = map.length / PAGE_SIZE;
        for (unsigned int offset = 0; offset < pages_count; offset++) {
            trace("index: %d/%d offset: %d/%d", index, kernel_maps_length,
                  offset, pages_count);
            char *vaddr = (char *)map.virtual_address + PAGE_SIZE * offset;
            char *physical_address =
                (char *)map.physical_address + PAGE_SIZE * offset;
            add_memory_mapping(vaddr, physical_address, page_directory,
                               page_directory != kernel_paging_directory);
            trace("index: %d/%d offset: %d/%d end", index, kernel_maps_length,
                  offset, pages_count);
        }
        debug("Mapping 0x%x -> 0x%x, %d pages, page_directory 0x%x",
              map.virtual_address, map.physical_address, pages_count,
              page_directory);
        debug("Mapping last page: 0x%x -> 0x%x",
              map.virtual_address + PAGE_SIZE * (pages_count - 1),
              map.physical_address + PAGE_SIZE * (pages_count - 1));
    }
}

void enable_kernel_paging() {
    // load kernel paging map to cr3(PDBR)
    int pdbr = (int)kernel_paging_directory;
    __asm__ __volatile__("movl %0, %%cr3\n\t" : : "r"(pdbr));

    // enable paging
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t" : "=r"(data) :);
    data = data | 0x80000000; // set PG bit
    __asm__ __volatile__("movl %0, %%cr0\n\t" : : "r"(data));

    debug("after enable PG bit");
    __asm__ __volatile__("jmp $0x0008, $flash_after_enable_paging\n\t" : :);

    __asm__ __volatile__("flash_after_enable_paging:\n\t");
    is_paging_enabled = true;
    return;
}

void disable_kernel_paging() {
    // disable paging
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t" : "=r"(data) :);
    data = data & (~(1 << 31)); // clear PG bit
    __asm__ __volatile__("movl %0, %%cr0\n\t" : : "r"(data));

    __asm__ __volatile__("jmp $0x0008, $flash_after_disable_paging\n\t" : :);
    __asm__ __volatile__("flash_after_disable_paging:\n\t");
    is_paging_enabled = false;
    return;
}

// return true on success
bool ksetup_kernel_paging() {
    prepare_kernel_paging();

    // write to physical address, should read back
    int *position1 = (int *)not_used_memory_start;
    int *position2 = (int *)(not_used_memory_start + PAGE_SIZE);
    int magic = 0x33667890;
    int value1 = magic;
    int value2 = 0;

    *position2 = value2;
    *position1 = value2;
    if (*position1 || *position2) {
        panic("prepare test failed");
    }
    *position2 = magic;
    if (*position2 != magic) {
        fatal("prepare test failed2");
    }
    // position1 = 0, position2 = magic

    add_kernel_memory_mapping(
        (void *)((int)position1 - (int)position1 % 0x1000),
        (void *)((int)position2 - (int)position2 % 0x1000)); // align to 4KB

    enable_kernel_paging();

    // check is paging enabled
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t" : "=r"(data) :);
    debug("cr0 PG bit should be 1: %d\n", data >> 31);

    // have written magic to position2, should be read back through posision1,
    // and *position2 should lead to page fault, because it's not mapped
    if (*position1 != magic) {
        printf("expected value %d but given %d\n", magic, *position1);
        panic("paging test failed");
    } else {
        debug("test paging OK\n");
        return true;
    }
}

void copy_process_mapping(void *start, uint32_t count) {
    const PTE *paging_directory = CURRENT_PROCESS->context.page_directory;
    // Just copy page dir entry, does it work?
    uint32_t addr = (uint32_t)start;
    uint32_t dir_index = addr >> 22;
    kernel_paging_directory[dir_index] = paging_directory[dir_index];

    if (count > (1024 * 1024 * 4)) {
        fatal("copy process mapping failed, count is too large");
    }
}