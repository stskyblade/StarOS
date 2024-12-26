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
                // space

                // copy first page
                constexpr uint32_t page_size = 1024 * 4; // 4KB
                uint8_t *page =
                    (uint8_t *)alloc_page(); // first page of this section
                auto offset_in_page =
                    phentry.p_vaddr & 0xFFF; // virtual address of that process
                // vaddr like 0x08048000, is the address of kernel .text section
                add_paging_map(
                    page,
                    page); // add kernel mapping of `page`, to edit content
                memset(page, 0, 1024 * 4);
                auto vpage =
                    (uint8_t *)(phentry.p_vaddr &
                                (~0xFFF)); // address of page in program mapping
                add_memory_mapping(vpage, page, p->paging_directory);
                debug("section mapping: 0x%x -> 0x%x", vpage, page);
                uint32_t bytes_to_copy = phentry.p_memsz;
                // first page, which is incomplete. only use one page
                if ((bytes_to_copy + offset_in_page) < page_size) {
                    memcpy(&page[offset_in_page], &buffer[phentry.p_offset],
                           bytes_to_copy);
                    bytes_to_copy = 0;
                } else {
                    // only copy first page
                    memcpy(&page[offset_in_page], &buffer[phentry.p_offset],
                           page_size - offset_in_page);
                    bytes_to_copy =
                        bytes_to_copy - (page_size - offset_in_page);
                }

                // copy following complete page
                // program vaddr of next page
                uint8_t *vaddr =
                    (uint8_t *)(phentry.p_vaddr & (~0xFFF)) + page_size;
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
    SegmentDescriptor *ldt = (SegmentDescriptor *)alloc_page();
    add_paging_map(ldt, ldt); // map ldt in kernel address, for edit
    SegmentDescriptor *vaddr =
        (SegmentDescriptor *)0x20000000; // LDT's start address at user space
    add_memory_mapping(
        vaddr, ldt,
        p->paging_directory); // map ldt in process space, for access
    p->ldt = vaddr;
    ldt[0] = {};
    ldt[1] = SegmentDescriptor(0, 0xfffff, 0xfa, 0xc); // process code segmet
    ldt[2] = SegmentDescriptor(0, 0xfffff, 0xfa, 0xc); // process data segment

    if (sizeof(SegmentDescriptor) != 8) {
        fatal("invalid SegmentDescriptor size: %d",
              (int)sizeof(SegmentDescriptor));
    }

    if (sizeof(TSS) != (26 * 4)) {
        fatal("invalid TSS size: %d", (int)sizeof(TSS));
    }

  // construct a TSS memory block
  TSS *tss = (TSS *)malloc(sizeof(TSS));
  tss->back_link = 0; // FIXME: back link
  tss->ss0 = 0x10;
  tss->ss1 = 0x10;
  tss->ss2 = 0x10;
  tss->esp0 = (uint32_t)malloc(1024 * 4) + 1024 * 4; // 4kb, kernel stack
  tss->esp1 = 0;
  tss->esp2 = 0;
  tss->cr3 = (uint32_t)p->paging_directory;
  tss->eip = header.e_entry;
  tss->eflags = 0x00000200; // copy from xv6-public
  tss->eax = 0;
  tss->ebx = 0;
  tss->ecx = 0;
  tss->edx = 0;
  tss->esp = 0x21000000; // FIXME:user stack, manual set, need add mapping
  uint32_t *stack_addr = (uint32_t *)alloc_page(); // 4kb user space stack
  add_paging_map(stack_addr,
                 stack_addr); // map stack in kernel address, for edit
  add_memory_mapping(
      (uint32_t *)tss->esp, stack_addr,
      p->paging_directory); // map ldt in process space, for access
  tss->esp += 1024 * 4;     // point to stack bottom

  tss->ebp = 0;
  tss->esi = 0;
  tss->edi = 0;

  uint16_t dpl_user = 0x3;
  uint16_t selector = (1 << 3) + (1 << 2) + dpl_user; // index = 1
  tss->cs = selector;
  selector = (2 << 3) + (1 << 2) + dpl_user; // index = 2
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
  extern SegmentDescriptor GDT[];
  extern int GDT_INDEX;
  extern int GDT_SIZE;
  uint32_t access_byte =
      9 + (0 << 5) + (1 << 7); // type=9 non busy, dpl=0, p=1. equals to 0x89,
                               // same as https://wiki.osdev.org/GDT_Tutorial
  uint32_t flags = 0b0001;     // g=0, avl=1
  GDT[GDT_INDEX++] = SegmentDescriptor((uint32_t)tss, sizeof(TSS) - 1,
                                       access_byte, flags); // task descriptor
  if (GDT_INDEX == GDT_SIZE) {
      fatal("size of GDT exceeds max limit")
  }

  // Task Gate Descriptor
  selector =
      ((GDT_INDEX - 1) << 3) + (0 << 2) + 0; // TI=0 for GDT, DPL=0 for kernel
  // 0x0000, 10000101 0x00
  // selector, 0x0000
  GDT[GDT_INDEX++] =
      SegmentDescriptor(selector & 0xffff, 0, 0x85, 0); // task gate descriptor
  // use iret to run Task Gate. read P313, TASK-RETURN
  // fake stack status, p159, with privilege transition, without error code
  // EFLAGS, 0xff old CS, old EIP
  // NT=1
  uint32_t reg = 0;
  __asm__ __volatile__("pushf\n\tpop %0\n\t" : "=r"(reg) :);
  reg = reg | (1 << 14); // set NT bit
  __asm__ __volatile__("push %0\n\tpopf\n\t" : : "r"(reg));
  // set Back Link in current TSS
  selector = ((GDT_INDEX - 1) << 3) + (0 << 2) +
             0; // TI=0 for GDT, DPL=0 for kernel. point to Task Gate
  // set TSS descriptor to busy
  GDT[GDT_INDEX - 2].type |= (1 << 1);
  // don't use stack, use TSS in TR->back_link

  uint16_t *ip = (uint16_t *)0x20200000;
  *ip = 0x28;
  *ip = 0x00;

  // stack layout after exception of interrupt, with privilege transition,
  // without error code old SS, old ESP, old EFLAGS, old CS, old EIP
  // SS=selector,ESP=?,EFLAGS=0,CS=another selector, EIP=e_entry
  uint32_t data = 0;
  data = tss->ss;
  __asm__ __volatile__("push %0\n\t" ::"r"(data));
  data = tss->esp;
  __asm__ __volatile__("push %0\n\t" ::"r"(data));
  data = tss->eflags;
  __asm__ __volatile__("push %0\n\t" ::"r"(data));
  data = tss->cs;
  __asm__ __volatile__("push %0\n\t" ::"r"(data));
  data = tss->eip;
  __asm__ __volatile__("push %0\n\t" ::"r"(data));

  KERNEL_TSS->back_link = selector; // FIXME: bug
  __asm__ __volatile__("debug_process:\n\t" ::);
  __asm__ __volatile__("iret\n\t" ::);

  int pdbr = (int)p->paging_directory;
  __asm__ __volatile__("movl %0, %%cr3\n\t" : : "r"(pdbr));
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
