#include "main.h"

void data_blocks_init (DATA_BLOCKS **data_blocks, int32_t count) {
    (*data_blocks) = malloc(sizeof(DATA_BLOCKS));
    (*data_blocks) -> length = count;

    for (int i = 0; i < (*data_blocks) -> length; i++) {
        (*data_blocks) -> directory[i] = malloc(sizeof(DIR_ITEM));
    }
}

DIR *get_drectory(VFS **vfs, char *dir_name) {
    for (int i = 0; i < (*vfs) -> data_blocks -> length; i++) {
        if (strcmp((*vfs) -> data_blocks -> directory[i] -> files[0] -> item_name, dir_name) == 0) {
            return (*vfs) -> data_blocks -> directory[i];
        }
    }
    return NULL;
}

int directory_exist (VFS **vfs, int32_t cluster_id, char *dir_name) {
    for (int i = 0; i < (*vfs) -> data_blocks -> directory[cluster_id] -> size; i++) {
        if (strcmp((*vfs) -> data_blocks -> directory[cluster_id] -> files[i] -> item_name, dir_name) == 0) {
            return 1;
        }
    }
    return 0;
}

void ls (VFS **vfs) {
//    get_drectory
    for (int i = 1; i < (*vfs) -> data_blocks -> directory[0] -> size; i++) {
        INODE *inode = get_inode_by_id(vfs, (*vfs) -> data_blocks -> directory[0] -> files[i] -> inode);
        if (inode -> isDirectory == 1) {
            printf("\t+ ");
        } else {
            printf("\t- ");
        }
        printf("%s\n", (*vfs) -> data_blocks -> directory[0] -> files[i] -> item_name);
    }
}