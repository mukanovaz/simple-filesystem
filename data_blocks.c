#include "main.h"

void data_blocks_init (DATA_BLOCKS **data_blocks, int32_t count) {
    (*data_blocks) = (DATA_BLOCKS*) malloc(sizeof(DATA_BLOCKS));
    (*data_blocks) -> length = count;

    for (int i = 0; i < (*data_blocks) -> length; i++) {
        (*data_blocks) -> directory[i] = (DIR*) malloc(sizeof(DIR));
    }
}

DIR *get_directory_by_name(VFS **vfs, char *dir_name) {
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
    char *array[10];
    int array_size = get_path_array((*vfs) -> actual_path, array);
    DIR *dir;
    dir = get_directory_by_name(vfs, array[array_size - 1]);
    if (dir == NULL) return;

    for (int i = 1; i < dir -> size; i++) {
        INODE *inode = get_inode_by_id(vfs, dir -> files[i] -> inode);
        if (inode -> isDirectory == 1) {
            printf("\t+ ");
        } else {
            printf("\t- ");
        }
        printf("%s\n", dir -> files[i] -> item_name);
    }
}

DIR *get_block_by_id (VFS **vfs, int32_t data_block_id) {
    for (int i = 0; i < (*vfs) -> data_blocks -> length; i++) {
        if ((*vfs) -> data_blocks -> directory[i] -> block_id  == data_block_id) {
            return (*vfs) -> data_blocks -> directory[i];
        }
    }
    return NULL;
}

void fwrite_data_block (VFS **vfs, int block_id) {
    fseek((*vfs) -> FILE, (*vfs) -> superblock -> data_start_address, SEEK_SET);
    fwrite((*vfs) -> data_blocks, sizeof(DATA_BLOCKS), 1, (*vfs) -> FILE);
    fflush((*vfs) -> FILE);

    int i;
    for (i = 0; i < (*vfs) -> superblock -> cluster_count; i++) {
        fwrite_directory(vfs, i);
    }
}

void fwrite_directory (VFS **vfs, int block_id) {
    fseek((*vfs) -> FILE, (*vfs) -> superblock -> data_start_address + sizeof(DATA_BLOCKS) + (block_id * sizeof(DIR)), SEEK_SET);
    fwrite((*vfs) -> data_blocks -> directory[block_id], sizeof(DIR), 1, (*vfs) -> FILE);
    fflush((*vfs) -> FILE);
    fwrite_files(vfs, block_id);
}

void fwrite_files (VFS **vfs, int block_id) {
    int j;
    for (j = 0; j < (*vfs) -> data_blocks -> directory[block_id] -> size; j++) {
        fseek((*vfs) -> FILE, (*vfs) -> superblock -> data_start_address + sizeof(DATA_BLOCKS) + (block_id * sizeof(DIR)) + (j * sizeof(DIR_ITEM)), SEEK_SET);
        fwrite((*vfs) -> data_blocks -> directory[block_id] -> files[j], sizeof(DIR_ITEM), 1, (*vfs) -> FILE);
        fflush((*vfs) -> FILE);
    }
}

void fread_data_blocks(VFS **vfs, FILE *file) {
    // Fill data block
    (*vfs) -> data_blocks = malloc(sizeof(DATA_BLOCKS));
    fseek(file, (*vfs) -> superblock -> data_start_address, SEEK_SET);
    fread((*vfs) -> data_blocks, sizeof(DATA_BLOCKS), 1, file);

    // Fill directories
    int i, j;
    for (i = 0; i < CLUSTER_COUNT; i++) {
        (*vfs) -> data_blocks -> directory[i] = malloc(sizeof(DIR));
        fseek(file, (*vfs) -> superblock -> data_start_address + sizeof(DATA_BLOCKS) + (i * sizeof(DIR)), SEEK_SET);
        fread((*vfs) -> data_blocks -> directory[i], sizeof(DIR), 1, file);
//         Fill files
        for (j = 0; j < (*vfs) -> data_blocks -> directory[i] -> size; j++) {
            (*vfs) -> data_blocks -> directory[i] -> files[j] = malloc(sizeof(DIR_ITEM));
            fseek(file, (*vfs) -> superblock -> data_start_address + sizeof(DATA_BLOCKS) + (i * sizeof(DIR)) + (j * sizeof(DIR_ITEM)), SEEK_SET);
            fread((*vfs) -> data_blocks -> directory[i] -> files[j], sizeof(DIR_ITEM), 1, file);
        }
    }
}