#include "kernel.h"

dir_t ROOT_DIR;
superblock_t SB;
bool isRootOpened = false; // true if dirblock of root directory is loaded

dir_entry_t direntry_area[1024]; // store opened direntry,used by readdir
ushort direntry_index = 0;

FILE opened_file_area[1024];
ushort opened_file_index = 0;

// load root dir to memory
void _init_root_dir() {
    if (isRootOpened) {
        return;
    }

    uchar buffer[512];
    read_sector(1, 1, buffer);
    memcopy((uchar *)&SB, buffer, sizeof(SB));

    read_sector(SB.dirblock_start, 1, buffer);
    memcopy(ROOT_DIR.data, buffer, sizeof(buffer));
    ROOT_DIR.index = 0;

    isRootOpened = true;
}

dir_t open_dir(const char *dir_path) {
    dir_t d;

    // start from root directory dir_t
    // recursively run through dir_path
    _init_root_dir();
    d = ROOT_DIR;

    uchar dir_entry_name[512];
    size_t i = 0;
    while (*dir_path) {
        if (*dir_path == '\0') {
            return d;
        } else if (*dir_path == '/') {
            dir_entry_name[i] = '\0';

            if (i == 0) {
                // root dir
                dir_path++;
                continue;
            }

            // find in current dir, and open
            dir_entry_t *ent;
            while ((ent = read_dir(d))) {
                if (strcmp((char *)ent->name, (char *)dir_entry_name)) {
                    // TODO;
                    uchar buffer[512];
                    read_sector(ent->addr, 1, buffer);
                    memcopy(d.data, buffer, 512);
                    d.index = 0;
                }
            }
            i = 0;
        } else {
            dir_entry_name[i] = *dir_path;
            i++;
        }

        dir_path++;
    }

    return d;
}

dir_entry_t *read_dir(dir_t &d) {
    // statically allocate memory for direntry
    dir_entry_t *dir = &(direntry_area[sizeof(dir_entry_t) * direntry_index]);
    memcopy((uchar *)dir, (uchar *)&(d.data[sizeof(dir_entry_t) * d.index]), sizeof(dir_entry_t));

    if (dir->name[0] == '\0') {
        // invalid dir entry
        dir = nullptr;
        return dir;
    }

    direntry_index++;
    d.index++;
    return dir;
}

void list_dir(const char *dir) {
    dir_t d;
    d = open_dir(dir);

    dir_entry_t *ent;
    ent = read_dir(d);
    while (ent) {
        print((char *)ent->name);
        print("\n");
        ent = read_dir(d);
    }
}

block_addr_t find_inode_of_pathname(const char *pathname) {
    // return inode number if found,  return 0 if failed
    // /hello.txt
    dir_t d;
    _init_root_dir();
    d = ROOT_DIR;

    uchar dir_entry_name[512];
    size_t i = 0;
    while (true) {
        if (*pathname == '/') {
            dir_entry_name[i] = '\0';
            if (i == 0) {
                pathname++;
                continue;
            }

            // is directory, find and open
            dir_entry_t *ent;
            while ((ent = read_dir(d))) {
                if (strcmp((char *)ent->name, (char *)dir_entry_name)) {
                    uchar buffer[512];
                    read_sector(ent->addr, 1, buffer);
                    memcopy(d.data, buffer, 512);
                    d.index = 0;
                }
            }
            i = 0;
        } else if (*pathname == '\0') {
            dir_entry_name[i] = '\0';

            dir_entry_t *ent;
            while ((ent = read_dir(d))) {
                if (strcmp((char *)ent->name, (char *)dir_entry_name)) {
                    return ent->addr;
                }
            }
            printf("\ncannot find file ");
            printf(pathname);
            printf("\n");
            break;
        } else if (*pathname) {
            // is alpha or num
            dir_entry_name[i] = *pathname;
            pathname++;
            i++;
        }
    }

    return 0;
}

FILE *fopen(const char *pathname) {
    // load inode of pathname to memory, and store it in FILE
    // statically allocated
    FILE *ptr = &opened_file_area[opened_file_index];

    // find inode of pathname
    block_addr_t inode_num = find_inode_of_pathname(pathname);
    if (!inode_num) {
        return nullptr;
    }
    // load into mem
    uchar buffer[512];
    read_sector(inode_num, 1, buffer);
    memcopy((uchar *)&ptr->inode, buffer, sizeof(inode_t));

    ptr->offset = 0;
    opened_file_index++;
    return ptr;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
    // read size * nmemb to ptr
    size_t bytes_to_read = size * nmemb;

    size_t current_block = f->offset / 512;

    uchar buffer[512];
    read_sector(f->inode.direct_addrs[current_block], 1, buffer);

    uchar *dest = (uchar *)ptr;

    // only need to read one block
    if ((bytes_to_read + f->offset) / 512 == current_block) {
        memcopy((uchar *)ptr, &buffer[f->offset], bytes_to_read);
        f->offset += bytes_to_read;
        return bytes_to_read;
    } else {
        // need read multi block
        size_t bytes_to_read_in_current_block = 512 - (f->offset % 512);
        memcopy((uchar *)ptr, &buffer[f->offset], bytes_to_read_in_current_block);
        f->offset = (current_block + 1) * 512;
        bytes_to_read -= bytes_to_read_in_current_block;
        dest += bytes_to_read_in_current_block;
        current_block++;

        while (bytes_to_read >= 512) {
            read_sector(f->inode.direct_addrs[current_block], 1, buffer);
            memcopy(dest, buffer, 512);
            current_block++;
            dest += 512;
            bytes_to_read -= 512;
            f->offset += 512;
        }

        if (bytes_to_read) {
            read_sector(f->inode.direct_addrs[current_block], 1, buffer);
            memcopy(dest, buffer, bytes_to_read);
            f->offset += bytes_to_read;
        }
        return size * nmemb;
    }

    return 0;
}
