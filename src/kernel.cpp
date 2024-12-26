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
    debug("Running kernel test:\n");
    test_malloc();
    debug("All test passed.\n");
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

alignas(
    PAGE_SIZE) char KERNEL_STACK_START[KERNEL_STACK_SIZE]; // 1MB kernel stack
char *const KERNEL_STACK_END = KERNEL_STACK_START + KERNEL_STACK_SIZE;

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
    GDT[1] = SegmentDescriptor(0, 0xfffff, 0x9a, 0xc); // kernel code segment
    GDT[2] = SegmentDescriptor(0, 0xfffff, 0x92, 0xc); // kernel data segment
    KERNEL_TSS = (TSS *)malloc(sizeof(TSS));

    GDT[3] = SegmentDescriptor((uint32_t)KERNEL_TSS, sizeof(TSS) - 1, 0x89, 0x1); // kernel TSS
    GDT_INDEX = 4;
    gdtr = {GDT_SIZE * 8, (uint32_t)GDT};
    // check GDTR has no padding
    if ((uint32_t)&gdtr.base - (uint32_t)&gdtr.limit != sizeof(uint16_t)) {
        fatal("padding in GDTR");
    }
    __asm__ __volatile__("lgdt (%0)\n\t" ::"r"(&gdtr));
    debug("change GDT success");

    // set Task Register
    uint16_t selector = (3 << 3) + (0 << 2) + 0;
    __asm__ __volatile__("ltr %0\n\r" ::"r"(selector));

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
