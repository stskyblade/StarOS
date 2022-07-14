// Convert files and directorys under ROOT_DIR to a disk img with my own filesystem.

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#define FILE_TYPE 1

namespace fs = std::filesystem;
using namespace std;

typedef unsigned int block_addr_t;

constexpr long long disk_size = 200 * 1024 * 1024; // in Bytes
long long block_amount = disk_size / 512;
unsigned char file_buf[disk_size];

block_addr_t next_datablock = 0;
block_addr_t next_inodeblock = 0;
block_addr_t next_dirblock = 0;

#define size_t unsigned int

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

short direntry_index = 0;
short max_direntry = 512 / sizeof(dir_entry_t);

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

    next_datablock = sb.datablock_start;
    next_inodeblock = sb.inodeblock_start;
    next_dirblock = sb.dirblock_start;
}

void package_root_directory() {
    // read files & directory under root directory, write to disk img
    for (const auto &entry : fs::directory_iterator(root_dir)) {
        cout << entry.path() << endl;

        if (entry.is_regular_file()) {
            // allocate inode
            inode_t i;
            i.size = std::filesystem::file_size(entry.path());
            int index = 0;

            // read file
            fstream f(entry.path());
            while (!f.eof()) {
                f.read((char *)&file_buf[next_datablock * 512], 512);
                i.direct_addrs[index] = next_datablock;
                cout << "write to index " << index << endl;
                next_datablock++;
                index++;
            }

            f.close();

            memcopy((unsigned char *)&file_buf[next_inodeblock * 512], (unsigned char *)&i, sizeof(i));

            // update dir entry
            auto p = entry.path();
            dir_entry_t dt;
            // dt.name = (unsigned char *)p.filename().c_str();
            memcopy((unsigned char *)&dt.name, (unsigned char *)(p.filename().c_str()), 20);
            dt.type = FILE_TYPE;
            dt.addr = next_inodeblock;
            cout << "addr: " << dt.addr << endl;
            next_inodeblock++;

            // FIXME: have bugs when read fs. file more than one will disappear.
            // file in different directory should have different dirblock
            memcopy((unsigned char *)&file_buf[next_dirblock * 512 + direntry_index * sizeof(dir_entry_t)], (unsigned char *)&dt, sizeof(dt));
            if ((direntry_index + 1) * sizeof(dir_entry_t) > 512) {
                next_dirblock++;
                direntry_index = 0;
            } else {
                direntry_index++;
            }
        }
    }
}

void update_bitmap() {
    // TODO:
}

int main() {
    init_low_level_format();
    cout << "Low level format success." << endl;
    init_superblock();
    cout << "Superblock written." << endl;

    // write directorys & files
    package_root_directory();

    update_bitmap();

    ofstream target_file;
    target_file.open(target_path, ios::out | ios::binary);
    target_file.write((const char *)file_buf, sizeof(file_buf));
    target_file.close();

    cout << "Make filesystem success." << endl;
    return 0;
}
