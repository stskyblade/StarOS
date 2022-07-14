#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define uchar uint8_t
#define ushort uint16_t
#define uint uint32_t
#define size_t uint32_t

typedef unsigned int block_addr_t;

struct superblock_t {
    block_addr_t bitmap_start;
    block_addr_t dirblock_start;
    block_addr_t inodeblock_start;
    block_addr_t datablock_start;
    block_addr_t end;
};

struct dir_t {
    // represent a directory in memory
    unsigned char data[512];
    size_t index; // used by readdir
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

struct FILE {
    // represent a file in memory
    inode_t inode;
    size_t offset; // read file will increse offset
};
