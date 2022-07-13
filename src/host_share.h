typedef unsigned int block_addr_t;

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
