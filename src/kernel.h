#pragma once
#include "system.h"
#include <stdint.h>

typedef uint32_t size_t;
typedef int pid_t;

// kernel space memory mappings: (keep same with kernel.ld)
// 0x40005000 - 0x4c805000: 200MB, text, text*。权限 AX, 读取和执行
// 0x4c805000 - 0x52c05000: 100MB, rodata。权限 A，仅读取
// 0x52c05000 - 0x59005000: 100MB: data, bss。权限 AW，读取和写入.

struct MemoryMap {
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t length; // in byte
};

constexpr uint32_t not_used_memory_start = 0x20200000; // to 0x40004000, 500MB
constexpr uint32_t text_memory_start = 0x40005000;
constexpr uint32_t text_memory_length = 0xc800000; // 200MB
constexpr uint32_t rodata_memory_start = text_memory_start + text_memory_length;
constexpr uint32_t rodata_memory_length = 0x6400000; // 100MB
constexpr uint32_t data_memory_start =
    rodata_memory_start + rodata_memory_length;
constexpr uint32_t data_memory_length = 0x6400000; // 100MB
constexpr uint32_t data_memory_end = data_memory_start + data_memory_length;
constexpr uint32_t free_memory_start = data_memory_end;
constexpr uint32_t free_memory_length = 1024 * 1024 * 500; // 500MB
// in order to alloc block larget than 4KB, alloc_page should break the
// continuous of free_memory
constexpr uint32_t free_memory_page_start =
    free_memory_start + free_memory_length;
constexpr uint32_t free_memory_page_length = 1024 * 1024 * 100; // 100MB
constexpr int kernel_maps_length = 6;
const MemoryMap Kernel_maps[kernel_maps_length] = {
    {0, 0, 1024 * 1024 * 4}, // map first 4MB to 4MB, including IO ports
    {text_memory_start, text_memory_start, text_memory_length},
    {rodata_memory_start, rodata_memory_start, rodata_memory_length},
    {data_memory_start, data_memory_start, data_memory_length},
    // free memory is mapped only used
    {free_memory_start, free_memory_start, free_memory_length},
    {free_memory_page_start, free_memory_page_start, free_memory_page_length},
};

// virtual address of User stack, stack bottom, 1MB
// need manual map
constexpr uint32_t USER_STACK_VADDRESS = 0x0ff00000;
constexpr uint32_t USER_STACK_SIZE = 1024 * 1024; // 1MB

const int PAGE_SIZE = 1024 * 4;
const int KERNEL_STACK_SIZE = 1024 * 1024;

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

extern PTE *kernel_paging_directory;
extern bool is_paging_enabled;
bool ksetup_kernel_paging();
void add_kernel_mappings(PTE *&page_directory);
void check_address_mapping(void *addr, const PTE *paging_directory);

// add normal memory mapping
void add_memory_mapping(void *linear_address, void *physical_address,
                        PTE *&paging_directory, bool user_level = true);
// add kernel memory mapping
void add_kernel_memory_mapping(void *linear_address, void *physical_address);

// copy memory mapping in process space [start, start+count) to kernel space
void copy_process_mapping(void *start, uint32_t count);
// ================== paging.cpp end ======================

// ================== interrupt.cpp start ======================
void init_interrupt_handler();
// ================== interrupt.cpp end ======================

// ================== process.cpp start ======================
template <class T> class LinkedList;

struct Context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t ss;
    uint32_t cs;
    uint32_t eip;
    uint32_t eflags;

    PTE *page_directory = nullptr;
};
enum ProcessStatus { Ready, Running, Blocking };
struct Process {
    int id;
    uint8_t *buffer = nullptr; // content of ELF file
    ProcessStatus status;
    Context context;
};
extern int allocated_process_id;
extern Process *CURRENT_PROCESS;
extern Process Kernel_proc;

extern LinkedList<Process *> ready_queue;
extern LinkedList<Process *> running_queue;
extern LinkedList<Process *> blocking_queue;
int execv(const char *pathname, char *const argv[]);
void switch_to_process(Process *p);
struct TrapFrame;
void restore_context_to_trapframe(Context &cxt, TrapFrame *tf);
void save_context_from_trapframe(Context &cxt, TrapFrame *tf);
// ================== process.cpp end ======================

// ================== kernel.cpp start ======================
enum { Kernel_thread, Process_thread, Interrupt_thread };
extern int Current_control_flow; // Kernel, Process, or Interrupt
int add_to_GDT(SegmentDescriptor d);
uint16_t descriptor_selector(uint16_t index, bool is_GDT, uint16_t RPL);
// ================== kernel.cpp start ======================

// in memory bytes, first field comes first, has lower memory address
// last field comes last, has higher memory address, should be pushed first
struct TrapFrame { // order should be opposite to alltraps.S
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t unused_esp; // status inside alltraps, useless
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t condition_code;
    uint32_t error_code;
    uint32_t return_addr; // EIP in page159
    uint32_t old_cs;
    uint32_t old_eflags;
    // below two fields only valid in interrupt with privilege transition
    uint32_t old_esp;
    uint32_t old_ss;
};

// ================== system_entry.cpp start ======================
void system_entry(int syscall_id, TrapFrame *tf);
extern bool gets_enabled; // indicate that a process is waiting for user input
extern int gets_count;    // indicate how many chars is process waiting for
extern int gets_already_count;
extern char *gets_buffer;
extern Process *process_waiting_gets; // the id of process waiting for input

struct CountDownClock {
    int64_t count_down;
    Process *process;
};
extern LinkedList<CountDownClock> waiting_sleep_queue;

// ================== system_entry.cpp end ======================
// ================== memory_management.cpp start ======================
struct MemoryBlock {
    size_t start; // including
    size_t end;   // excluding
    size_t size;
    bool is_valid;
};
extern MemoryBlock Free_blocks[];
extern int Blocks_count;
void *malloc(size_t size);
void free(void *);
// ================== memory_management.cpp end ======================
// ================== schedular.cpp start ======================
void schedular();
// ================== schedular.cpp end ======================
// ================== PIC.cpp start ======================
#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define EOI_CMD 0x20

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

// PIT timer ports
#define CHANNEL_0_DATA_PORT 0x40
#define CHANNEL_1_DATA_PORT 0x41
#define CHANNEL_2_DATA_PORT 0x42
#define PIT_COMMAND_PORT 0x43

void init_PIC();
// ================== PIC.cpp end ======================
// ================== ps2_keyboard.cpp start ======================
constexpr uint8_t KEYBOARD_DATA_PORT = 0x60;

struct KeyboardKey {
    int id;
    char *name;
};
struct KeyEvent {
    int event_id;
    int key_id;
    char *keyname;
    bool is_pressed;
};
extern KeyboardKey Key_table[];
extern KeyEvent Keyevent_table[];
extern bool Key_pressed_table[];
void ps2_keyboard_interrupt();
// ================== ps2_keyboard.cpp end ======================
#include "linked_list.h"
// ================== sleep.cpp start ======================
extern int64_t Count_down;
void ksleep(uint32_t seconds);
// ================== sleep.cpp end ======================