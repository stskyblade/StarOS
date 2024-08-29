// This is part of bootloader, but in 32-bit
// This code runs in 32bit protected mode, privilege 0, kernel mode(not in kernel)
// This code is loaded to memory address 0x7cxx, by bootloader.S

#include "bootloader32.h"

void print_data_type_size() {
    printf("Size of char 0x%x\n", sizeof(char));
    printf("Size of short 0x%x\n", sizeof(short));
    printf("Size of int 0x%x\n", sizeof(int));
    printf("Size of long 0x%x\n", sizeof(long));
    printf("Size of long long 0x%x\n", sizeof(long long));
}

extern "C" {

// never return
// entry
// jump from bootloader.S:jmp_32
void bootloader32_start() {
    // TODO: print hello,C. bootloader32
    // print_data_type_size();
    char *s = "hello,C. bootloader32.\n";
    printf(s);

    struct stat filestatus;
    const char *kernel_filename = "/staros_kernel.elf";
    stat(kernel_filename, &filestatus);
    uint32_t kernel_size = filestatus.st_size;

    // 3GB = 1024 * 1024 * 1024 * 3 = 0xC0000000
    // load kernel content to 3GB
    uint8_t *buffer = (uint8_t *)(1024U * 1024 * 1024 * 3);
    for (uint32_t i = 0; i < 32; i++) {
        buffer[i] = i;
    }

    printf("memory at 0xc0000000:\n");
    print_memory(buffer, 32); // all is zeros

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
                memcpy((uint8_t *)phentry.p_vaddr, (uint8_t *)&buffer[phentry.p_offset], phentry.p_memsz);
                printf("Memory at %d after copy:\n", phentry.p_vaddr);
                print_memory((uint8_t *)phentry.p_vaddr, 80);
            }
        }
    } else {
        panic("Unsupported kernel file");
    }

    memset(buffer, 0, kernel_size);
    printf("Waiting...\n");
    for (uint64_t i = 0; i < 0xffffffff; i++) { // 12 seconds on laptop; 20 seconds on Qemu
        /* code */
    }

    printf("Call kernel entry\n");
    void (*kernel_entry)() = (void (*)())header.e_entry;
    kernel_entry();

    while (1) {
        ;
    }
}
}
