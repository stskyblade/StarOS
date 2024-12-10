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

const int memory_layout_table_num = 80;
extern Memory_layout_entry memory_map_table[memory_layout_table_num];

void print_memory_layout() {
    for (int i = 0; i < memory_layout_table_num; i++) {
        Memory_layout_entry entry = memory_map_table[i];

        if (!entry.type) {
            break;
        } else {
            const char *msg = "Unknown type";
            // Type 1: Usable (normal) RAM
            // Type 2: Reserved - unusable
            // Type 3: ACPI reclaimable memory
            // Type 4: ACPI NVS memory
            // Type 5: Area containing bad memory
            switch (entry.type) {
            case 1:
                msg = "Usable";
                break;
            case 2:
                msg = "Reserved";
                break;
            case 3:
                msg = "ACPI recl";
                break;
            case 4:
                msg = "ACPI NVS";
                break;
            case 5:
                msg = "bad memory";
                break;
            default:
                break;
            }
            uint64_t end = entry.base + entry.length;
            uint32_t start_high = entry.base >> 32;
            uint32_t start_low = entry.base & 0xFFFFFFFF;
            uint32_t end_high = end >> 32;
            uint32_t end_low = end & 0xFFFFFFFF;
            uint32_t length_high = entry.length >> 32;
            uint32_t length_low = entry.length & 0xFFFFFFFF;

            printf("#%d | 0x%x%x | 0x%x%x | 0x%x%x | %s\n", i, start_high, start_low, end_high, end_low, length_high, length_low, msg);
        }
    }
}

extern "C" {

// never return
// entry
// jump from bootloader.S:jmp_32
// jump to kernel_main
void bootloader32_start() {
    // TODO: print hello,C. bootloader32
    // print_data_type_size();
    info("Enter C source code of bootloader32...        OK");

    struct stat filestatus;
    const char *kernel_filename = "/staros_kernel.elf";
    stat(kernel_filename, &filestatus);
    uint32_t kernel_size = filestatus.st_size;

    // load kernel content to 0x20200000, about 500MB free space
    uint8_t *buffer = (uint8_t *)(0x20200000);
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
                // printf("Memory at 0x%x after copy:\n", phentry.p_vaddr);
                // print_memory((uint8_t *)phentry.p_vaddr, 80);
            }
        }
    } else {
        panic("Unsupported kernel file");
    }

    // enter kernel_main, never return
    void (*kernel_entry)() = (void (*)())header.e_entry;
    kernel_entry();
}
}
