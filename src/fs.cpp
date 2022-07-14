#include "kernel.h"

dir_t ROOT_DIR;
superblock_t SB;
bool isRootOpened = false; // true if dirblock of root directory is loaded

dir_entry_t direntry_area[1024]; // store opened direntry,used by readdir
ushort direntry_index = 0;

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
    print("open_dir start\n");
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
    print("You should see content of / directory below:\n");

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
