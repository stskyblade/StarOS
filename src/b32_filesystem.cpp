#include "bootloader32.h"

enum DEVICE_TYPE { PCI_IDE };

enum FILESYSTEM_TYPE { FAT32 };

struct PARTITION {
    DEVICE_TYPE t;
    int device_number;    // count from 0
    int partition_number; // count from 0
};

// like a line in /etc/fstab:
// /dev/sdb1     fat3    /
struct FS_entry {
    const char *mount_point;
    FILESYSTEM_TYPE filesystem_type;
    PARTITION partition;
};

const int fstab_size = 10;
bool fstab_initialized = false;
FS_entry FS_TABLE[fstab_size]; // represents all mounted device

void init_fstab() {
    if (fstab_initialized) {
        return;
    }
    PARTITION p{PCI_IDE, 0, 0};
    FS_entry fs{"/", FAT32, p};
    FS_TABLE[0] = fs;
    fstab_initialized = true;
}

FS_entry match_fs(const char *filename) {
    return FS_TABLE[0];
}

const int MAX_FILENAME_SIZE = 256;

// represent status of a file: name, size, location...
// abstract dir entry, include long name part
struct DIR_entry {
    uint8_t name[MAX_FILENAME_SIZE + 1];
    uint32_t first_cluster_number;
    uint32_t size; // size of file in bytes
};

// part of directory content in FAT32
// exists in Data Area
// 32 bytes
struct physical_dir_entry {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t created_time_seconds;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t last_accessed_date;
    uint16_t first_cluster_number_high;
    uint16_t last_modify_time;
    uint16_t last_modify_date;
    uint16_t first_cluster_number_low;
    uint32_t size; // size of file in bytes
} __attribute__((packed));

// 32 bytes
// part of directory content in FAT32
// represent long filename
struct physical_long_filename_entry {
    uint8_t order;
    uint16_t chars1[5];
    uint8_t attribute;
    uint8_t type;
    uint8_t short_name_checksum;
    uint16_t chars2[6];
    uint16_t zeros;
    uint16_t chars3[2];
} __attribute__((packed));

// part of Master Boot Record partition table
// exists in MBR sector of disk
// information about a partition: where, size, type
struct MBR_PARTITION_ENTRY {
    uint8_t status;
    uint8_t start_CHS1;
    uint8_t start_CHS2;
    uint8_t start_CHS3;
    uint8_t partition_type;
    uint8_t end_CHS1;
    uint8_t end_CHS2;
    uint8_t end_CHS3;
    uint32_t lba;  // LBA of partition's first sector
    uint32_t size; // Number of sectors of partition
} __attribute__((packed));

// https://wiki.osdev.org/FAT#Boot_Record
// first sector of partition which is FAT32 filesystem type
// including how to find the Data Area, FAT Area
struct PBR {
    uint8_t jmp1;
    uint8_t jmp2;
    uint8_t jmp3;

    uint64_t oem_identifier;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_FAT;
    uint16_t num_root_directory_entries;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_FAT_16;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors; // LBA of first sector of partition
    uint32_t large_sector_count;

    // FAT32
    uint32_t sectors_per_FAT_32;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t cluster_number_root_directory;
    uint16_t sector_number_FSInfo;
    uint16_t sector_number_backup_boot;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint8_t drive_number;
    uint8_t NT_flags;
    uint8_t signature;
    uint32_t serial_number;
    uint64_t label1;
    uint16_t label2;
    uint8_t label3;
    uint64_t identifier;
    // boot code
    // bootable signature 0xAA55
} __attribute__((packed));

// TODO: when a cluster has more than one sector
void read_cluster(int directory_cluster_num, uint8_t *dest_buffer, PARTITION p, PBR pbr) {
    // cluster number -> sector number
    uint64_t fat_size = pbr.sectors_per_FAT_16 == 0 ? pbr.sectors_per_FAT_32 : pbr.sectors_per_FAT_16;
    uint64_t root_dir_sectors = 0; // FAT32, this will be zero
    // relative num to first sector of partition
    uint64_t first_data_sector = pbr.num_reserved_sectors + (pbr.num_FAT * fat_size) + root_dir_sectors;
    uint64_t first_sector_of_given_cluster = (directory_cluster_num - 2) * pbr.sectors_per_cluster + first_data_sector; // root directory sector is cluster 2, the first data sector
    uint64_t absolute_sector_num = first_sector_of_given_cluster + pbr.hidden_sectors;
    read_disk_sector(absolute_sector_num, 1, dest_buffer);
}

// search file in given directory recursively
// return the DIR_entry of that file
// `filename` like "local/bin/hello.txt"
// TODO:
// filename is longer than MAX_FILENAME_SIZE
// filename is encoding other than ASCII
// directory is larger than one sector
// does return struct correctly copy array inside struct
DIR_entry locate_file_dirent(int directory_cluster_num, const char *filename, PARTITION p, PBR pbr) {
    // splite current part from filename
    char current_part_filename[MAX_FILENAME_SIZE + 1];
    for (int i = 0; i < MAX_FILENAME_SIZE; i++) {
        if (filename[i] == '/') {
            current_part_filename[i] = '\0';
            filename = filename + i + 1; // rest of filename
            break;
        } else {
            current_part_filename[i] = filename[i];
        }
    }

    // for dir entry in directory
    uint8_t buffer[512];
    read_cluster(directory_cluster_num, buffer, p, pbr);
    // directory content
    physical_dir_entry *table = (physical_dir_entry *)buffer;

    DIR_entry abstract_entry;
    for (unsigned int i = 0; i < (512 / sizeof(physical_dir_entry)); i++) {
        physical_dir_entry entry = table[i];
        if (entry.attributes == 0x0f) {
            // is long name entry
            // 0x42, 0x01
            // copy filename part to abstract_entry
            physical_long_filename_entry lfn = *(physical_long_filename_entry *)&entry;
            int order = lfn.order & 0xf;
            int offset = (order - 1) * 13; // 13 chars for each lfn entry

            // null terminated, 0xff padding
            for (int i = 0; i < 13; i++) {
                uint16_t c;
                if (i < 5) {
                    c = lfn.chars1[i];
                } else if (i < 11) {
                    c = lfn.chars2[i - 5];
                } else {
                    // i < 13
                    c = lfn.chars3[i - 11];
                }

                if (c != 0xffff) {
                    // valid two-bytes char, include 0x0000
                    abstract_entry.name[offset] = c & 0xff;
                    offset++;
                }
            }
        } else {
            // normal dir entry
            abstract_entry.first_cluster_number = (entry.first_cluster_number_high << 16) + entry.first_cluster_number_low;
            abstract_entry.size = entry.size;

            // compare to filename
            if (!strcmp(current_part_filename, (const char *)abstract_entry.name)) {
                // filename equal
                if (!(entry.attributes & 0x10)) {
                    // is file, rather than directory
                    return abstract_entry;
                } else {
                    // is directory
                    return locate_file_dirent(abstract_entry.first_cluster_number, filename, p, pbr);
                }
            }
        }
    }
}

// return next cluster num on the chain
// FIXME: cluster has more than one sector
// FIXME: stack overflow
uint32_t next_cluster(uint32_t cluster_num, PBR pbr) {
    // first fat sector
    uint32_t first_fat_sector = pbr.num_reserved_sectors;
    uint32_t fat_size = pbr.sectors_per_FAT_16 == 0 ? pbr.sectors_per_FAT_32 : pbr.sectors_per_FAT_16;

    // read one sector is enough
    // 16 entrys per sector
    // 0-15 read first
    // 16-31 read second
    uint32_t sector_offset = cluster_num / 16;
    uint8_t buffer[512];
    read_disk_sector(first_fat_sector + sector_offset + pbr.hidden_sectors, 1, buffer);

    // table value
    uint32_t *FAT = (uint32_t *)buffer;
    return FAT[cluster_num % 16];
}

// return 0 on success, others for error
int fat32_read(const char *filename, uint8_t *dest_buffer, FS_entry fs) {
    // remove mount point part from filename
    const char *p1 = filename;
    const char *p2 = fs.mount_point;
    while (*p1 && *p2) {
        p1++;
        p2++;
    }
    // p1 points to rest part of filename
    // example:
    // p1 = dir1/dir2/hello.txt
    // p1 = /dir1/dir2/hello.txt
    if (*p1 == '/') {
        p1++;
    }

    uint8_t buffer[512];
    // MBR data
    read_disk_sector(0, 1, buffer);
    MBR_PARTITION_ENTRY mbr = *(MBR_PARTITION_ENTRY *)&buffer[0x1be];
    // PBR
    read_disk_sector(mbr.lba, 1, buffer);
    PBR pbr = *(PBR *)buffer;

    int cluster_num = 2; // TODO: root directory cluster
    DIR_entry e = locate_file_dirent(cluster_num, p1, fs.partition, pbr);
    int file_cluster_num = e.first_cluster_number;
    uint32_t file_size = e.size;
    uint32_t bytes_to_read = file_size;

    while (bytes_to_read) {
        if (bytes_to_read < pbr.bytes_per_sector) {
            // less than one block
            uint8_t tmp[512];
            read_cluster(file_cluster_num, tmp, fs.partition, pbr);
            for (unsigned int i = 0; i < bytes_to_read; i++) {
                dest_buffer[i] = tmp[i];
            }
            break;
        } else {
            // TODO: cluster has more than one sector
            read_cluster(file_cluster_num, dest_buffer, fs.partition, pbr);
            dest_buffer += pbr.bytes_per_sector;
            file_cluster_num = next_cluster(file_cluster_num, pbr);
            bytes_to_read -= pbr.bytes_per_sector;
        }
    }
    return 0;
}

int fs_read(const char *filename, uint8_t *buffer) {
    init_fstab();
    FS_entry f = match_fs(filename);

    if (!(f.filesystem_type == FAT32 && f.partition.t == PCI_IDE)) {
        return -1; // not supported FS or device
    }

    if (f.filesystem_type == FAT32) {
        return fat32_read(filename, buffer, f);
    }

    // not supported filesystem
    return -1;
}