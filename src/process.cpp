#include "bootloader32.h"
#include "kernel.h"

struct Process {
};

int execv(const char *pathname, char *const argv[]) {
    // read program into memory
    struct stat filestatus;
    stat(pathname, &filestatus); // /testadd
    uint32_t size = filestatus.st_size;
    // FIXME: allocate memory for program, malloc should be revieded under paging enabled
    uint8_t *buffer = (uint8_t *)malloc(size);
    memset(buffer, 0, size);
    fs_read(pathname, buffer);

    debug("read program into memory success");
    // prepare section memory mapping, paging table
    // construct a struct process
    // switch to program
    return 0;
}