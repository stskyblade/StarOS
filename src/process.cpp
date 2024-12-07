#include "bootloader32.h"
#include "kernel.h"

struct Process {
    PTE *paging_directory = nullptr;
    SegmentDescriptor *ldt;
    uint8_t *buffer = nullptr; // content of ELF file
};

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
    // in order to edit page mapping of process,
    // need to map the memory which page_directory occupies in kernel mapping
    add_paging_map(p->paging_directory, p->paging_directory);
    memset(p->paging_directory, 0, 1024 * 4);

    // for each section in program,
    // copy content to a new page, keep offset
    // TODO: add mapping to paging_directory of process

    // just map data section & text section
    // prepare section memory mapping, paging table
    // switch to program
    ELF_HEADER header = *(ELF_HEADER *)buffer;
    const bool is_valid_elf = header.magic_num == 0x464c457f && header.bitness == 1 && header.endianness == 1 && header.version == 1 && header.ABI == 0 && header.ABI_version == 0 && header.type == 0x02 && header.machine == 0x03 && header.e_version == 1;
    if (is_valid_elf) {
        // valid elf format file
        Program_header *program_header_table = (Program_header *)&buffer[header.e_phoff];
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

                // copy section from program file buffer to program address space

                // copy first page
                constexpr uint32_t page_size = 1024 * 4;       // 4KB
                uint8_t *page = (uint8_t *)alloc_page();       // first page of this section
                auto offset_in_page = phentry.p_vaddr & 0xFFF; // virtual address of that process
                // vaddr like 0x08048000, is the address of kernel .text section
                add_paging_map(page, page); // add kernel mapping of `page`, to edit content
                memset(page, 0, 1024 * 4);
                auto vpage = (uint8_t *)(phentry.p_vaddr & (~0xFFF)); // address of page in program mapping
                add_memory_mapping(vpage, page, p->paging_directory);
                uint32_t bytes_to_copy = phentry.p_memsz;
                // first page, which is incomplete. only use one page
                if ((bytes_to_copy + offset_in_page) < page_size) {
                    memcpy(&page[offset_in_page], &buffer[phentry.p_offset], bytes_to_copy);
                    bytes_to_copy = 0;
                } else {
                    // only copy first page
                    memcpy(&page[offset_in_page], &buffer[phentry.p_offset], page_size - offset_in_page);
                    bytes_to_copy = bytes_to_copy - (page_size - offset_in_page);
                }

                // copy following complete page
                // program vaddr of next page
                uint8_t *vaddr = (uint8_t *)(phentry.p_vaddr & (~0xFFF)) + page_size;
                // program content of next page
                uint8_t *src_addr = &buffer[page_size];
                while (bytes_to_copy) {
                    page = (uint8_t *)alloc_page();
                    add_paging_map(page, page); // same as add_paging_map above
                    add_memory_mapping(vaddr, page, p->paging_directory);
                    memset(page, 0, 1024 * 4);
                    if (bytes_to_copy >= page_size) {
                        // copy next page
                        memcpy(page, src_addr, page_size);
                        bytes_to_copy -= page_size;
                        src_addr += page_size;
                    } else {
                        // copy last page, incomplete
                        // less than one page
                        memcpy(page, src_addr, bytes_to_copy);
                        break;
                    }
                }
            }
        }
    } else {
        fatal("Unsupported program file");
    }
    debug("load program to memory, done");

    // setup LDT and memory segment
    // LDT[0] empty
    // LDT[1] code segment
    // LDT[2] data segment
    // https://wiki.osdev.org/GDT_Tutorial
    SegmentDescriptor *ldt = (SegmentDescriptor *)malloc(1024 * 4);
    add_paging_map(ldt, ldt); // map ldt in kernel address, for edit
    SegmentDescriptor *vaddr = (SegmentDescriptor *)0x20000000;
    add_memory_mapping(vaddr, ldt, p->paging_directory); // map ldt in process space, for access
    p->ldt = vaddr;
    ldt[0] = {};
    ldt[1] = {
        0xffff, // limit low
        0x0,    // base low
        0x0,    // base mid
        // 0xFA, for user code segment
        0b1010, // type
        0b1,    // s
        0b11,   // dpl
        0b1,    // p
        0xf,    // limit_high
        // 0xC, 0b1100
        0,   // avl
        0,   // o
        1,   // x
        1,   // g
        0x0, // base_high
    };
    ldt[2] = {
        0xffff, // limit low
        0x0,    // base low
        0x0,    // base mid
        // 0xF2, for user data segment
        0b0010, // type
        0b1,    // s
        0b11,   // dpl
        0b1,    // p
        0xf,    // limit_high
        // 0xC, 0b1100
        0,   // avl
        0,   // o
        1,   // x
        1,   // g
        0x0, // base_high
    };

    if (sizeof(SegmentDescriptor) != 8) {
        fatal("invalid SegmentDescriptor size: %d", (int)sizeof(SegmentDescriptor));
    }

    if (sizeof(TSS) != (26 * 4)) {
        fatal("invalid TSS size: %d", (int)sizeof(TSS));
    }

    // construct a TSS memory block
    TSS *tss = (TSS *)malloc(sizeof(TSS));
    tss->back_line = 0; // FIXME: backline
    tss->ss0 = 0x10;
    tss->ss1 = 0x10;
    tss->ss2 = 0x10;
    tss->esp0 = (uint32_t)malloc(1024 * 4); // 4kb
    tss->esp1 = 0;
    tss->esp2 = 0;
    tss->cr3 = (uint32_t)p->paging_directory;
    tss->eip = header.e_entry;
    tss->eflags = 0; // FIXME
    tss->eax = 0;
    tss->ebx = 0;
    tss->ecx = 0;
    tss->edx = 0;
    tss->esp = 0;
    tss->ebp = 0;
    tss->esi = 0;
    tss->edi = 0;

    uint16_t selector = 1 << 3 + 1 << 2 + 0; // index = 1
    tss->cs = selector;
    selector = 2 << 3 + 1 << 2 + 0; // index = 2
    tss->ds = selector;
    tss->es = selector;
    tss->fs = selector;
    tss->gs = selector;
    tss->ss = selector;
    tss->ldt = 0x0010; // kernel data segment
    tss->t = 0;
    tss->io_map_base = 0; // FIXME

    tss->reserved = 0;
    tss->reserved0 = 0;
    tss->reserved1 = 0;
    tss->reserved2 = 0;
    tss->reserved3 = 0;
    tss->reserved4 = 0;
    tss->reserved5 = 0;
    tss->reserved6 = 0;
    tss->reserved7 = 0;
    tss->reserved8 = 0;
    tss->reserved9 = 0;
    tss->reserved10 = 0;

    // TODO: construct a TSS descriptor
    // TODO: create a GDT array in C++ source code

    int pdbr = (int)p->paging_directory;
    __asm__ __volatile__("debug_process:\n\t" ::);
    __asm__ __volatile__("movl %0, %%cr3\n\t"
                         :
                         : "r"(pdbr));
    // execute
    // should just add to ready queue, waiting for the Schedular to run the process
    // to debug, jump to process entry directly
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
