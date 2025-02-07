#include "bootloader32.h"
#include "kernel.h"

Process *CURRENT_PROCESS = nullptr;

int execv(const char *pathname, char *const argv[]) {
    // read program into memory
    struct stat filestatus;
    stat(pathname, &filestatus); // /testadd
    uint32_t size = filestatus.st_size;
    uint8_t *buffer = (uint8_t *)malloc(size);
    memset(buffer, 0, size);
    fs_read(pathname, buffer);
    debug("read program into memory success");

    // construct Process struct
    Process *p = (Process *)malloc(sizeof(Process));
    p->paging_directory = (PTE *)alloc_page();
    p->buffer = buffer;
    memset(p->paging_directory, 0, 1024 * 4);

    // for each section in program,
    // copy content to a new page, keep offset

    // just map data section & text section
    // prepare section memory mapping, paging table
    // switch to program
    ELF_HEADER header = *(ELF_HEADER *)buffer;
    const bool is_valid_elf = header.magic_num == 0x464c457f &&
                              header.bitness == 1 && header.endianness == 1 &&
                              header.version == 1 && header.ABI == 0 &&
                              header.ABI_version == 0 && header.type == 0x02 &&
                              header.machine == 0x03 && header.e_version == 1;
    if (is_valid_elf) {
        // valid elf format file
        Program_header *program_header_table =
            (Program_header *)&buffer[header.e_phoff];
        // section like .text, .data
        for (uint32_t i = 0; i < header.e_phnum; i++) {
            Program_header phentry = program_header_table[i];
            if (phentry.p_filesz != phentry.p_memsz) {
                panic("Unsupported Program header entry %d.", i);
            } else {
                if (!phentry.p_memsz) {
                    // sections like .debug_* shouldn't be loaded into memory
                    continue;
                }

                // copy section from program file buffer to program address
                // space:

                // copy first page
                uint8_t *page =
                    (uint8_t *)alloc_page(); // first page of this section
                auto offset_in_page =
                    phentry.p_vaddr & 0xFFF; // virtual address of that process
                // vaddr like 0x08048000, is the address of kernel .text section
                memset(page, 0, PAGE_SIZE);
                auto vpage =
                    (uint8_t *)(phentry.p_vaddr &
                                (~0xFFF)); // address of page in program mapping
                // add process mapping of `page`
                add_memory_mapping(vpage, page, p->paging_directory, true);
                debug("section mapping: 0x%x -> 0x%x", vpage, page);
                uint32_t bytes_to_copy = phentry.p_memsz;
                // first page, which is incomplete. only use one page
                if ((bytes_to_copy + offset_in_page) < PAGE_SIZE) {
                    memcpy(&page[offset_in_page], &buffer[phentry.p_offset],
                           bytes_to_copy);
                    bytes_to_copy = 0;
                } else {
                    // only copy first page
                    memcpy(&page[offset_in_page], &buffer[phentry.p_offset],
                           PAGE_SIZE - offset_in_page);
                    bytes_to_copy =
                        bytes_to_copy - (PAGE_SIZE - offset_in_page);
                }

                // copy following complete page
                // program vaddr of next page
                uint8_t *vaddr =
                    (uint8_t *)(phentry.p_vaddr & (~0xFFF)) + PAGE_SIZE;
                // program content of next page
                uint8_t *src_addr = &buffer[PAGE_SIZE];
                while (bytes_to_copy) {
                    page = (uint8_t *)alloc_page();
                    add_memory_mapping(vaddr, page, p->paging_directory, true);
                    memset(page, 0, PAGE_SIZE);
                    if (bytes_to_copy >= PAGE_SIZE) {
                        // copy next page
                        memcpy(page, src_addr, PAGE_SIZE);
                        bytes_to_copy -= PAGE_SIZE;
                        src_addr += PAGE_SIZE;
                    } else {
                        // copy last page, incomplete
                        // less than one page
                        memcpy(page, src_addr, bytes_to_copy);
                        break;
                    }
                }
            }
        }

        // TODO: execute initialization functions
    } else {
        fatal("Unsupported program file");
    }
    debug("load program to memory, done");

    if (sizeof(SegmentDescriptor) != 8) {
        fatal("invalid SegmentDescriptor size: %d",
              (int)sizeof(SegmentDescriptor));
    }

    if (sizeof(TSS) != (26 * 4)) {
        fatal("invalid TSS size: %d", (int)sizeof(TSS));
    }

    // mapping program stack pages
    // -1MB ~ USER_STACK_VADDRESS
    for (uint32_t offset = 0; offset < USER_STACK_SIZE; offset += PAGE_SIZE) {
        char *page = (char *)alloc_page();
        // map in process space
        add_memory_mapping((char *)USER_STACK_VADDRESS - PAGE_SIZE - offset,
                           page, p->paging_directory, true);
    }

    add_kernel_mappings(p->paging_directory);

    // for debug
    // check if some addresses be mapped in process space
    // check_address_mapping((char *)p->paging_directory, p->paging_directory);
    // check_address_mapping((char *)0x52d07fa8, p->paging_directory);

    // use iret to run Task Gate. read P313, TASK-RETURN
    // fake stack status, p159, with privilege transition, without error code
    // stack layout after exception of interrupt, with privilege transition,
    // without error code old SS, old ESP, old EFLAGS, old CS, old EIP
    // SS=selector,ESP=?,EFLAGS=0,CS=another selector, EIP=e_entry

    // setup memory segment descriptor in GDT
    int code_segment_index = add_to_GDT(
        SegmentDescriptor(0, 0xfffff, 0xfa, 0xc)); // process code segmet
    int data_segment_index = add_to_GDT(
        SegmentDescriptor(0, 0xfffff, 0xf2, 0xc)); // process data segmet

    // each one should be 4 bytes
    uint32_t data = 0;
    uint16_t selector = descriptor_selector(data_segment_index, true, 3);
    // set data segment registers
    __asm__ __volatile__("mov %0, %%ds\n\t" ::"r"(selector));
    __asm__ __volatile__("mov %0, %%es\n\t" ::"r"(selector));
    __asm__ __volatile__("mov %0, %%fs\n\t" ::"r"(selector));
    __asm__ __volatile__("mov %0, %%gs\n\t" ::"r"(selector));

    data = selector;
    debug("SS selector: 0x%x", data);
    __asm__ __volatile__("pushl %0\n\t" ::"r"(data)); // SS segment selector
    data = USER_STACK_VADDRESS - 4;                   // ESP
    __asm__ __volatile__("pushl %0\n\t" ::"r"(data));
    data = 0; // Eflags
    // __asm__ __volatile__("pushl %0\n\t" ::"r"(data));
    __asm__ __volatile__("pushf\n\t" ::); // EFLAGS
    selector = descriptor_selector(code_segment_index, true, 3);
    data = selector; // CS segment
    debug("CS selector: 0x%x", data);
    __asm__ __volatile__("pushl %0\n\t" ::"r"(data));
    data = header.e_entry; // EIP
    __asm__ __volatile__("pushl %0\n\t" ::"r"(data));

    CURRENT_PROCESS = p;
    // switch to process address space
    int pdbr = (int)p->paging_directory;
    __asm__ __volatile__("movl %0, %%cr3\n\t" : : "r"(pdbr));
    info("Entering ring3: ") __asm__ __volatile__("debug_process:\n\t" ::);
    __asm__ __volatile__("iret\n\t" ::);

    // execute
    // should just add to ready queue, waiting for the Schedular to run the
    // process to debug, jump to process entry directly
    // __asm__ __volatile__("movl %0, %%eax\n\t"
    // :
    // : "r"(header.e_entry));
    // FIXME: infinite reboot
    __asm__ __volatile__("jmp $0x0008, $0x8048054\n\t" ::);

    int a = 3;
    int b = 4;
    int c = a + b;
    int d = c + 338;
    void (*program_entry)() = (void (*)())header.e_entry;
    program_entry();
    return 0;
}
