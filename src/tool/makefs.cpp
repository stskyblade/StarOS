// Convert files and directorys under ROOT_DIR to a disk img with my own filesystem.

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using namespace std;

typedef unsigned int block_addr_t;

constexpr long long disk_size = 200 * 1024 * 1024; // in Bytes
long long block_amount = disk_size / 512;
unsigned char file_buf[disk_size];

struct superblock_t {
    block_addr_t bitmap_start;
    block_addr_t dirblock_start;
    block_addr_t inodeblock_start;
    block_addr_t datablock_start;
    block_addr_t end;
};

struct dir_entry_t {
    unsigned char name[20];
    unsigned char type;
    block_addr_t addr;
};

struct inode_t {
    size_t size;
    block_addr_t direct_addrs[10];
};

string target_path = "/home/stskyblade/Project/StarOS/rootfs.img";
string root_dir = "/home/stskyblade/Project/StarOS/root_dir";

void init_low_level_format() {

    long long l = disk_size;
    const char c = 0x00;
    while (l--) {
        file_buf[l - 1] = 0x00;
    }
}

void memcopy(unsigned char *target, unsigned char *source, size_t count) {
    for (size_t i = 0; i < count; i++) {
        target[i] = source[i];
    }
}

void init_superblock() {
    // count from 0, 0 means MBR, 1 superblock, 2, bitmap
    superblock_t sb;
    sb.bitmap_start = 2;
    sb.dirblock_start = sb.bitmap_start + ceil(block_amount / 8.0 / 512.0);
    sb.inodeblock_start = sb.dirblock_start + 100;
    sb.datablock_start = sb.inodeblock_start + 100;
    sb.end = block_amount - 1;

    memcopy((unsigned char *)&file_buf[512], (unsigned char *)&sb, sizeof(sb));
}

void package_root_directory() {
    // read files & directory under root directory, write to disk img
    for (const auto &entry : fs::directory_iterator(root_dir)) {
        cout << entry.path() << endl;

        if (entry.is_file()) {
            // TODO:
            auto p = entry.path();
            dir_entry_t dt;
            dt.name = p.filename();
            dt.type = FILE_TYPE;
            dt.addr =
        }
    }
}

int main() {
    init_low_level_format();
    cout << "Low level format success." << endl;
    init_superblock();
    cout << "Superblock written." << endl;

    // write directorys & files
    package_root_directory();

    ofstream target_file;
    target_file.open(target_path, ios::out | ios::binary);
    target_file.write((const char *)file_buf, sizeof(file_buf));
    target_file.close();

    cout << "Make filesystem success." << endl;
    return 0;
}
