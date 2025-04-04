#include "bootloader32.h"
#include "kernel.h"
#include "linked_list.h"

Process *CURRENT_PROCESS = nullptr;
int allocated_process_id = 1;
// must include linked_list if you want to use below vairables
LinkedList<Process *> ready_queue;
LinkedList<Process *> running_queue;
LinkedList<Process *> blocking_queue;

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
    zeromem(p, sizeof(Process));
    p->id = allocated_process_id;
    allocated_process_id++;
    p->context.page_directory = (PTE *)alloc_page();
    p->buffer = buffer;
    p->status = Ready;
    memset(p->context.page_directory, 0, 1024 * 4);

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
                add_memory_mapping(vpage, page, p->context.page_directory,
                                   true);
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
                    add_memory_mapping(vaddr, page, p->context.page_directory,
                                       true);
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
                           page, p->context.page_directory, true);
    }

    add_kernel_mappings(p->context.page_directory);

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
    int segment_index;
    segment_index = add_to_GDT(
        SegmentDescriptor(0, 0xfffff, 0xfa, 0xc)); // process code segmet
    p->context.cs = descriptor_selector(segment_index, true, 3);
    segment_index = add_to_GDT(
        SegmentDescriptor(0, 0xfffff, 0xf2, 0xc)); // process data segmet
    p->context.ds = descriptor_selector(segment_index, true, 3);
    p->context.es = descriptor_selector(segment_index, true, 3);
    p->context.fs = descriptor_selector(segment_index, true, 3);
    p->context.gs = descriptor_selector(segment_index, true, 3);
    p->context.ss = descriptor_selector(segment_index, true, 3);
    p->context.esp = USER_STACK_VADDRESS - 4;
    p->context.eip = header.e_entry;

    ready_queue.push_back(p);
    return 0;
}

// Save current Context of kernel, including page_directory, registers
// EIP is left empty
// When call this function, EAX and ESP, EBP has been modified
inline Context save_context() {
    uint32_t data = 0;
    // save eax before use it
    __asm__ __volatile__("mov %%eax, %0\n\t" : "=m"(data) :);

    // construct a Context in reverse order
    __asm__ __volatile__("mov %%cr3, %%eax\n\t" ::);
    __asm__ __volatile__("push %%eax\n\t" ::); // value of cr3

    __asm__ __volatile__("pushf\n\t" ::); // eflags

    __asm__ __volatile__("push %0\n\t" ::"i"(0)); // leave EIP empty

    __asm__ __volatile__("push %%cs\n\t" ::);
    __asm__ __volatile__("push %%ss\n\t" ::);
    __asm__ __volatile__("push %%ds\n\t" ::);
    __asm__ __volatile__("push %%es\n\t" ::);
    __asm__ __volatile__("push %%fs\n\t" ::);
    __asm__ __volatile__("push %%gs\n\t" ::);
    __asm__ __volatile__("pusha\n\t" ::);
    Context *p = nullptr;
    __asm__ __volatile__("mov %%esp, %0\n\t" : "=m"(p) :);
    p->eax = 0; // invalid value, it has been modified outside
    Context result = *p;
    // restore stack pointer
    __asm__ __volatile__("add %0, %%esp\n\t" ::"i"(sizeof(Context)));
    return result;
}

Process Kernel_proc;

void switch_to_process(Process *p) {
    Context kernel_context = save_context();
    __asm__ __volatile__("mov %%esp, %0" : "=m"(kernel_context.esp) :);
    __asm__ __volatile__("mov %%ebp, %0" : "=m"(kernel_context.ebp) :);
    extern int scheduler_restore_here;
    kernel_context.eip = (int)&scheduler_restore_here;
    KERNEL_TSS->esp0 = kernel_context.esp;
    // save current context of kernel
    Kernel_proc.context = kernel_context;

    // each one should be 4 bytes
    uint32_t data = 0;
    Context &cxt = p->context;

    // set data segment registers
    __asm__ __volatile__("mov %0, %%ds\n\t" ::"r"(cxt.ds));
    __asm__ __volatile__("mov %0, %%es\n\t" ::"r"(cxt.es));
    __asm__ __volatile__("mov %0, %%fs\n\t" ::"r"(cxt.fs));
    __asm__ __volatile__("mov %0, %%gs\n\t" ::"r"(cxt.gs));

    // prepare data for IRET
    __asm__ __volatile__("pushl %0\n\t" ::"r"(cxt.ss));  // SS segment selector
    __asm__ __volatile__("pushl %0\n\t" ::"r"(cxt.esp)); // ESP
    __asm__ __volatile__("pushf\n\t" ::);             // EFLAGS
    __asm__ __volatile__("pushl %0\n\t" ::"r"(cxt.cs));  // CS
    __asm__ __volatile__("pushl %0\n\t" ::"r"(cxt.eip)); // EIP

    // switch to process address space
    __asm__ __volatile__("movl %0, %%cr3\n\t" : : "r"(cxt.page_directory));
    debug("Entering ring3: ");
    __asm__ __volatile__("debug_process:\n\t" ::);
    Current_control_flow = Process_thread;

    // restore general registers
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"r"(cxt.eax));
    __asm__ __volatile__("mov %0, %%ebx\n\t" ::"r"(cxt.ebx));
    __asm__ __volatile__("mov %0, %%ecx\n\t" ::"r"(cxt.ecx));
    __asm__ __volatile__("mov %0, %%edx\n\t" ::"r"(cxt.edx));
    __asm__ __volatile__("mov %0, %%esi\n\t" ::"r"(cxt.esi));
    __asm__ __volatile__("mov %0, %%edi\n\t" ::"r"(cxt.edi));
    __asm__ __volatile__("mov %0, %%ebp\n\t" ::"r"(cxt.ebp));
    __asm__ __volatile__("iret\n\t" ::);

    // return from interrupt
    __asm__ __volatile__("scheduler_restore_here:\n\t" ::);
    Current_control_flow = Kernel_thread;
}

void restore_context_to_trapframe(Context &cxt, TrapFrame *tf) {
    tf->eax = cxt.eax;
    tf->ebx = cxt.ebx;
    tf->ecx = cxt.ecx;
    tf->edx = cxt.edx;
    tf->esi = cxt.esi;
    tf->edi = cxt.edi;
    // tf->esp = cxt.esp; keep it
    tf->ebp = cxt.ebp;
    tf->ds = cxt.ds;
    tf->es = cxt.es;
    tf->fs = cxt.fs;
    tf->gs = cxt.gs;
    tf->return_addr = cxt.eip;
    tf->old_cs = cxt.cs;
    tf->old_ss = cxt.ss;
    tf->old_esp = cxt.esp;
    tf->old_eflags = cxt.eflags;
    int pdbr = (int)cxt.page_directory;
    __asm__ __volatile__("movl %0, %%cr3\n\t" : : "r"(pdbr));
}

void save_context_from_trapframe(Context &cxt, TrapFrame *tf) {
    // uint32_t edi;
    cxt.edi = tf->edi;
    // uint32_t esi;
    cxt.esi = tf->esi;
    // uint32_t ebp;
    cxt.ebp = tf->ebp;
    // uint32_t esp;
    cxt.esp = tf->old_esp;
    // uint32_t ebx;
    cxt.ebx = tf->ebx;
    // uint32_t edx;
    cxt.edx = tf->edx;
    // uint32_t ecx;
    cxt.ecx = tf->ecx;
    // uint32_t eax;
    cxt.eax = tf->eax;
    // uint32_t gs;
    cxt.gs = tf->gs;
    // uint32_t fs;
    cxt.fs = tf->fs;
    // uint32_t es;
    cxt.es = tf->es;
    // uint32_t ds;
    cxt.ds = tf->ds;

    // FIXME: need to distinguish from interrupt from kernel space or user
    // space uint32_t ss;
    cxt.ss = tf->old_ss;
    // uint32_t cs;
    cxt.cs = tf->old_cs;
    // uint32_t eip;
    cxt.eip = tf->return_addr;
    // uint32_t eflags;
    cxt.eflags = tf->old_eflags;
}