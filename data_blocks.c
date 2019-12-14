#include "main.h"

void data_blocks_init (DATA_BLOCKS **data_blocks, int32_t count) {
    (*data_blocks) = malloc(sizeof(DATA_BLOCKS));
    (*data_blocks) -> length = count;
    (*data_blocks) -> directory = calloc((*data_blocks) -> length, sizeof(DIR));
}

int directory_exist (VFS **vfs, int32_t cluster_id, char *dir_name) {
    if (strcmp((*vfs) -> data_blocks -> directory[cluster_id] -> files[0] -> item_name, dir_name) == 0) {
        return 1;
    }
    return 0;
}

void ls (VFS **vfs) {
    for (int i = 0; i < (*vfs) -> data_blocks -> directory[0] -> size; i++) {
        printf("\t%s: %d\n", (*vfs) -> data_blocks -> directory[0] -> files[i] -> item_name,
               (*vfs) -> data_blocks -> directory[0] -> files[i] -> inode);
    }
}