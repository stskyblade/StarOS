#include "bootloader32.h"
#include "kernel.h"

PTE *kernel_paging_directory = nullptr;
bool is_paging_enabled = false;

void add_paging_map(void *linear_address, void *physical_address) {
    return add_memory_mapping(linear_address, physical_address, kernel_paging_directory);
}

// add a Page Table Entry
// map `linear_address` to `physical_address`, 4KB
// FIXME:linear_address provide DIR index & PTE index. So page table is not added to directory at index 0, 1, 2...
// but index comes from linear_address. 1024 entries match 10-bit dir, 10-bit page
void add_memory_mapping(void *linear_address, void *physical_address, PTE *&paging_directory) {
    // debug("page mapping 0x%x -> 0x%x", (uint32_t)linear_address, (uint32_t)physical_address);
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

    bool new_allocated_paging_directory = false;
    if (!paging_directory) {
        // create a new page directory
        paging_directory = reinterpret_cast<PTE *>(alloc_page());
        new_allocated_paging_directory = true;
        trace("paging_directory: 0x%x\n", (int)paging_directory);
    }

    PTE &dir_entry = paging_directory[reinterpret_cast<uint32_t>(linear_address) >> 22];
    int dir_entry_offset = reinterpret_cast<uint32_t>(linear_address) >> 22;
    trace("dir_entry_offset: 0x%x\n", dir_entry_offset);
    bool new_allocated_page_table = false;
    if (!dir_entry.p) { // invalid dir_entry
        // allocate a new page table, and set up dir_entry
        PTE *page_table = reinterpret_cast<PTE *>(alloc_page());

        new_allocated_page_table = true;

        dir_entry.p = 1;
        dir_entry.rw = 1;
        dir_entry.user_or_supervisor = 0;
        dir_entry.reserved1 = 0;
        dir_entry.access = 0;
        dir_entry.dirty = 0;
        dir_entry.reserved2 = 0;
        dir_entry.avail = 0;
        dir_entry.address = reinterpret_cast<uint32_t>(page_table) >> 12; // save high 20bit
    }

    // use dir_entry.address, page part in linear_address to calculate the location of page table entry
    PTE *page_table = reinterpret_cast<PTE *>(reinterpret_cast<uint32_t>(dir_entry.address) << 12);
    trace("page_table: 0x%x\n", (int)page_table);
    int entry_offset = reinterpret_cast<uint32_t>(linear_address) >> 12 & 0b1111111111;
    trace("entry_offset: 0x%x\n", entry_offset);
    PTE &entry = page_table[reinterpret_cast<uint32_t>(linear_address) >> 12 & 0b1111111111]; // middle 10bit
    if (new_allocated_page_table) {
        add_paging_map(page_table, page_table);
    }
    entry.p = 1;
    entry.rw = 1;
    entry.user_or_supervisor = 0;
    entry.reserved1 = 0;
    entry.access = 0;
    entry.dirty = 0;
    entry.reserved2 = 0;
    entry.avail = 0;
    entry.address = reinterpret_cast<uint32_t>(physical_address) >> 12; // save high 20bit

    if (new_allocated_paging_directory) {
        trace("new paging directory -> ");
        add_memory_mapping(paging_directory, paging_directory, paging_directory);
    }

    if (new_allocated_page_table) {
        trace("new page table -> ");
        add_memory_mapping(page_table, page_table, paging_directory);
    }
}

// set up page tables, linear memory address map to same physical memory address
// first 4MB -> 4MB
// map STACK to same address, range from 0x12D00000 to 0x20000000
// map 00100000 -> 00100000
// map 00101000 -> 00101000
// map 00102000 -> 00102000
// map 00103000 -> 00103000
// map 0c900000 -> 0c900000
// map 12d00000 -> 12d00000
void prepare_kernel_paging() {
    // map first 4MB
    for (int i = 0; i < 1024; i++) {
        add_paging_map((void *)(i << 12), (void *)(i << 12));
    }

    // map STACK to same address, range from 0x12D00000 to 0x20000000, about 500mb
    for (int i = 0x12D00; i < 0x20000; i++) {
        add_paging_map((void *)(i << 12), (void *)(i << 12));
    }

    // read memory section table from kernel file
    struct stat filestatus;
    const char *kernel_filename = "/staros_kernel.elf";
    stat(kernel_filename, &filestatus);
    uint32_t kernel_size = filestatus.st_size;

    uint8_t *buffer = (uint8_t *)malloc(kernel_size);
    memset(buffer, 0, kernel_size);
    fs_read(kernel_filename, buffer);
    ELF_HEADER header = *(ELF_HEADER *)buffer;

    if (header.magic_num == 0x464c457f && header.bitness == 1 && header.endianness == 1 && header.version == 1 && header.ABI == 0 && header.ABI_version == 0 && header.type == 0x02 && header.machine == 0x03 && header.e_version == 1 && header.e_phnum == 3) {
        // valid kernel
        Program_header *program_header_table = (Program_header *)&buffer[header.e_phoff];
        for (uint32_t i = 0; i < header.e_phnum; i++) {
            Program_header phentry = program_header_table[i];
            if (phentry.p_filesz != phentry.p_memsz) {
                panic("Unsupported Program header entry %d.", i);
            } else {
                // is a valid memory section, like .text, .data
                uint32_t segment_size = phentry.p_memsz;
                uint32_t page_frame_size = 1024 * 4; // 4KB
                uint8_t *base = (uint8_t *)phentry.p_vaddr;

                // map all pages in the Section to same address
                while (segment_size > page_frame_size) {
                    add_paging_map(base, base);
                    segment_size = segment_size - page_frame_size;
                    base = base + page_frame_size;
                }
                add_paging_map(base, base);
            }
        }
    } else {
        panic("Unsupported kernel file");
    }
}

void enable_kernel_paging() {
    // load kernel paging map to cr3(PDBR)
    int pdbr = (int)kernel_paging_directory;
    __asm__ __volatile__("movl %0, %%cr3\n\t"
                         :
                         : "r"(pdbr));

    // enable paging
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t"
                         : "=r"(data)
                         :);
    data = data | 0x80000000; // set PG bit
    __asm__ __volatile__("movl %0, %%cr0\n\t"
                         :
                         : "r"(data));

    debug("after enable PG bit");
    __asm__ __volatile__("jmp $0x0008, $flash_after_enable_paging\n\t"
                         :
                         :);

    __asm__ __volatile__("flash_after_enable_paging:\n\t");
    is_paging_enabled = true;
    return;
}

void disable_kernel_paging() {
    // disable paging
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t"
                         : "=r"(data)
                         :);
    data = data & (~(1 << 31)); // clear PG bit
    __asm__ __volatile__("movl %0, %%cr0\n\t"
                         :
                         : "r"(data));

    __asm__ __volatile__("jmp $0x0008, $flash_after_disable_paging\n\t"
                         :
                         :);
    __asm__ __volatile__("flash_after_disable_paging:\n\t");
    is_paging_enabled = false;
    return;
}

// return true on success
bool ksetup_kernel_paging() {
    prepare_kernel_paging();

    // write to physical address 0x50005100, should read back
    int *position1 = (int *)0x40005100;
    int *position2 = (int *)0x50005100;
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

    add_paging_map((void *)((int)position1 - (int)position1 % 0x1000),
                   (void *)((int)position2 - (int)position2 % 0x1000)); // align to 4KB

    enable_kernel_paging();

    // check is paging enabled
    uint32_t data;
    __asm__ __volatile__("movl %%cr0, %0\n\t"
                         : "=r"(data)
                         :);
    debug("cr0 PG bit should be 1: %d\n", data >> 31);

    // have written magic to position2, should be read back through posision1,
    // and *position2 should lead to page fault
    if (*position1 != magic) {
        printf("expected value %d but given %d\n", magic, *position1);
        panic("paging test failed");

    } else {
        debug("test paging OK\n");
        return true;
    }
}
