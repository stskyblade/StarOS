#include "kernel.h"
#include "bootloader32.h"

void test_malloc() {
    int first_addr =
        (int)free_memory_start + sizeof(TSS); // kernel.cpp allocate a TSS
    int *ip = (int *)malloc(sizeof(int));
    if (ip != (int *)first_addr) {
        panic("malloc failed 1");
    }

    int *ip2 = (int *)malloc(sizeof(int));
    if (ip2 != (int *)(first_addr + 8)) { // first int allocate 8 bytes
        panic("malloc failed 2");
    }

    char *cp = (char *)malloc(25);
    if (cp != (char *)(first_addr + 16)) { // last int allocate 8 bytes
        panic("malloc failed 3");
    }

    char *cp2 = (char *)malloc(1);
    if (cp2 != (char *)(first_addr + 16 + 25 + 7)) { // last allocate 25 bytes, padding 7 bytes
        panic("malloc failed 4");
    }

    printf(".");
}

void run_test() {
    debug("Running kernel test:");
    test_malloc();
    debug("All test passed.");
}

#define PRINT_SIZE(type) printf("Size of [" #type "] is: %d\n", sizeof(type))
void test_int_size() {
    // qemu:
    // Size of [char] is: 1
    // Size of [short] is: 2
    // Size of [int] is: 4
    // Size of [long] is: 4
    // Size of [long long] is: 8
    // Size of [float] is: 4
    // Size of [double] is: 8
    // Size of [void *] is: 4

    PRINT_SIZE(char);
    PRINT_SIZE(short);
    PRINT_SIZE(int);
    PRINT_SIZE(long);
    PRINT_SIZE(long long);
    PRINT_SIZE(float);
    PRINT_SIZE(double);
    PRINT_SIZE(void *);
}

extern const int GDT_SIZE = 512;
SegmentDescriptor GDT[GDT_SIZE];
GDTR gdtr;
int GDT_INDEX = 0; // one past last descriptor
TSS *KERNEL_TSS = 0;
constexpr int Kernel_code_segment_desc_index = 1;
constexpr int Kernel_data_segment_desc_index = 2;
constexpr int Kernel_tss_segment_desc_index = 3;

int add_to_GDT(SegmentDescriptor d) {
    if (GDT_INDEX >= GDT_SIZE) {
        fatal("GDT exceeds limit: %d", GDT_INDEX);
    }
    int index = GDT_INDEX;
    GDT[GDT_INDEX] = d;
    GDT_INDEX++;
    return index;
}

// construct a selector of descriptor in GDT or LDT
// https://wiki.osdev.org/Segment_Selector
uint16_t descriptor_selector(uint16_t index, bool is_GDT, uint16_t RPL) {
    uint16_t ti = is_GDT ? 0 : 1;
    return (index << 3) + (ti << 1) + (RPL & 0b11);
}

alignas(PAGE_SIZE) char KERNEL_STACK_START[KERNEL_STACK_SIZE]; // 1MB kernel
                                                               // stack
char *const KERNEL_STACK_END = KERNEL_STACK_START + KERNEL_STACK_SIZE - 4;

extern "C" {
void kernel_main() {
    // init stack
    __asm__ __volatile__("mov %0, %%esp\n\t" : : "m"(KERNEL_STACK_END));

    info("Enter kernel...       OK");
    info("Welcome to StarOS, by stskyblade");
    info("=====================================================================");
    info("There must be another me in this world, doing what I dare not do and living the life I want to live.");
    info("=====================================================================");
    // test_int_size();

    // init GDT, replace GDT in mbr
    // https://wiki.osdev.org/GDT_Tutorial
    GDT[0] = {0, 0, 0, 0};
    GDT[Kernel_code_segment_desc_index] =
        SegmentDescriptor(0, 0xfffff, 0x9a, 0xc); // kernel code segment
    GDT[Kernel_data_segment_desc_index] =
        SegmentDescriptor(0, 0xfffff, 0x92, 0xc); // kernel data segment
    KERNEL_TSS = (TSS *)malloc(sizeof(TSS));
    memset(KERNEL_TSS, 0, sizeof(TSS));
    uint16_t selector =
        descriptor_selector(Kernel_data_segment_desc_index, true, 0);
    if (selector != 0x0010) {
        fatal("invalid data selector: 0x%x", selector);
    }

    KERNEL_TSS->ss0 = selector;
    KERNEL_TSS->esp0 = (uint32_t)KERNEL_STACK_END; // maybe wrong

    // value comes from https://wiki.osdev.org/Getting_to_Ring_3
    GDT[Kernel_tss_segment_desc_index] =
        SegmentDescriptor((uint32_t)KERNEL_TSS, sizeof(TSS), 0x89,
                          0x0); // kernel TSS

    GDT_INDEX = 4;
    gdtr = {GDT_SIZE * 8, (uint32_t)GDT};
    // check GDTR has no padding
    if ((uint32_t)&gdtr.base - (uint32_t)&gdtr.limit != sizeof(uint16_t)) {
        fatal("padding in GDTR");
    }
    __asm__ __volatile__("lgdt (%0)\n\t" ::"r"(&gdtr));
    __asm__ __volatile__("jmp $0x0008, $flush_gdt\n\t" ::);
    __asm__ __volatile__("flush_gdt:\n\t" ::);
    uint32_t data = selector;
    __asm__ __volatile__("push %%eax\n\t" ::);
    __asm__ __volatile__("mov %0, %%eax\n\t" ::"r"(data));
    __asm__ __volatile__("mov %%eax, %%ds\n\t" ::);
    __asm__ __volatile__("mov %%eax, %%es\n\t" ::);
    __asm__ __volatile__("mov %%eax, %%fs\n\t" ::);
    __asm__ __volatile__("mov %%eax, %%gs\n\t" ::);
    __asm__ __volatile__("mov %%eax, %%ss\n\t" ::);
    __asm__ __volatile__("pop %%eax\n\t" ::);
    debug("change GDT success");
    // flush_tss();

    // setup Task Register
    selector = (Kernel_tss_segment_desc_index * 8) | 0;
    // __asm__ __volatile__("mov %0, %%ax\n\t" ::"r"(selector));
    __asm__ __volatile__("ltr %0\n\t" ::"r"(selector));
    debug("change TR success");

    // init interrupt handlers
    init_interrupt_handler();

    run_test();

    // init memory paging
    bool success = ksetup_kernel_paging();
    if (success) {
        info("Enable paging...      OK");
    } else {
        fatal("Enable paging...     Failed");
    }

    // execute a user program
    char *const arg1 = "arg1";
    char *const argv[1] = {arg1};
    execv("/testadd", argv);

    // never return
    while (1) {
        sleep(1);
        printf(".");
        ;
    }
}
}
