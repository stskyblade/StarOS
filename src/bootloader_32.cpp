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
void bootloader32_start() {
    // TODO: print hello,C. bootloader32
    // print_data_type_size();
    char *s = "hello,C. bootloader32.\n";
    printf(s);
    print_memory_layout();

    // uint8_t buffer[512];
    // read_disk_sector(8386, 1, buffer);
    // print_memory(buffer, 160);

    // char *filename = "/hello.txt";
    // struct stat filestatus;
    // uint8_t buffer1[512];
    // stat(filename, &filestatus);
    // printf("%s %dbytes:\n", filename, filestatus.st_size);
    // fs_read(filename, buffer1);
    // print_memory(buffer1, 32);

    // // panic("panic %d\n", 828366412);

    // filename = "/staros_kernel.elf";
    // stat(filename, &filestatus);
    // printf("%s %dbytes:\n", filename, filestatus.st_size);
    // uint8_t buffer2[filestatus.st_size];
    // fs_read(filename, buffer2);
    // print_memory(buffer2, 32);

    // filename = "/large_hello.txt";
    // stat(filename, &filestatus);
    // printf("%s %dbytes:\n", filename, filestatus.st_size);
    // uint8_t buffer3[512 * 8];
    // fs_read(filename, buffer3);
    // print_memory(buffer3 + 1000, 160);

    while (1) {
        ;
    }
}
}
