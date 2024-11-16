#include "kernel.h"
#include "bootloader32.h"

void test_malloc() {
    int *ip = (int *)malloc(sizeof(int));
    if (ip != (int *)0x20200000) {
        panic("failed");
    }

    int *ip2 = (int *)malloc(sizeof(int));
    if (ip2 != (int *)0x20200008) { // first int allocate 8 bytes
        panic("failed");
    }

    char *cp = (char *)malloc(25);
    if (cp != (char *)(0x20200008 + 8)) { // last int allocate 8 bytes
        panic("failed");
    }

    char *cp2 = (char *)malloc(1);
    if (cp2 != (char *)(0x20200008 + 8 + 25 + 7)) { // last allocate 25 bytes, padding 7 bytes
        panic("failed");
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

extern "C" {
void kernel_main() {
    // init stack
    uint8_t *STACK_BOTTOM = (uint8_t *)0x20000000 - 1024 * 1024; // leave 1MB free space
    __asm__ __volatile__("mov %0, %%esp\n\t"
                         :
                         : "m"(STACK_BOTTOM));

    info("Enter kernel...       OK");
    info("Welcome to StarOS, by stskyblade");
    info("=====================================================================");
    info("There must be another me in this world, doing what I dare not do and living the life I want to live.");
    info("=====================================================================");
    // test_int_size();

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
