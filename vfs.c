#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include "main.h"

void vfs_init(VFS **vfs, char *filename, size_t disk_size) {
    printf("INFO: Creating virtual file-system based on inode\n");
    // Fill filename
    (*vfs) = (VFS*) calloc(1, sizeof(VFS));
    (*vfs) -> filename = calloc(MAX_NAME_LEN, sizeof(char));

    strcpy((*vfs) -> filename, filename);

    // Actual path
    (*vfs) -> actual_path = malloc((strlen(filename) + 1) * sizeof(char));
    strcpy((*vfs) -> actual_path, "/root");

    // Open .dat file
    FILE *vfs_file = fopen(filename, "r+");
    if (vfs_file == NULL) {
        // Create new file
        printf("INFO: File .dat not found. Creating new .dat file..\n");
        (*vfs) -> FILE = fopen((*vfs) -> filename, "w+");

        // Create Superblock
        SUPERBLOCK *superblock;
        superblock_init(&superblock, disk_size, ONE_CLUSTER_SIZE);
        (*vfs) -> superblock = superblock;

        // Create bitmap
        BITMAP *bitmap;
        bitmap_init(&bitmap, (*vfs) -> superblock -> cluster_count);
        (*vfs) -> bitmap = bitmap;

        // Create Data blocks
        DATA_BLOCKS *dataBlocks;
        data_blocks_init(&dataBlocks, (*vfs) -> superblock -> cluster_count);
        (*vfs) -> data_blocks = dataBlocks;

        // Create inode blocks
        inode_blocks_init(vfs);

        create_vfs_file(vfs);

    } else {
        printf("INFO: Preparing file system..\n");
        fread_superblock(vfs, vfs_file);
        fread_inode_block(vfs, vfs_file);
        fread_bitmap(vfs, vfs_file);
        fread_data_blocks(vfs, vfs_file);
        fclose(vfs_file);
        (*vfs) -> FILE = fopen((*vfs) -> filename, "r+");
    }

}

void create_vfs_file(VFS **vfs) {
    (*vfs) -> FILE = fopen((*vfs) -> filename, "r+");
    if ((*vfs) -> FILE == NULL) {
        printf("File %s not found\n", (*vfs) -> filename);
        return;
    }
    else {
        // Write Superblock
        fwrite((*vfs) -> superblock, sizeof(SUPERBLOCK), 1, (*vfs) -> FILE);
        fflush((*vfs) -> FILE);

        // Write bitmap
        fseek((*vfs) -> FILE, (*vfs) -> superblock -> bitmap_start_address, SEEK_SET);
        fwrite((*vfs) -> bitmap -> data, sizeof(unsigned char), (*vfs) -> bitmap -> length, (*vfs) -> FILE);
        fflush((*vfs) -> FILE);

        fseek((*vfs) -> FILE, (*vfs) -> superblock -> data_start_address, SEEK_SET);
    }
}

void set_path_to_root(VFS **vfs) {
    memset((*vfs) -> actual_path, 0, PATH_MAX);
}

void go_to_parent_folder(VFS **vfs) {
    int i, length = get_folder_count((*vfs) -> actual_path);

    char *tok = strtok((*vfs) -> actual_path, "/");
    char path[PATH_MAX];
    memset(path, 0, PATH_MAX);

    for (i = 0; i < length - 1; i++) {
        strcat(path, "/");
        strcat(path, tok);
        tok = strtok(NULL, "/");
    }

    set_path_to_root(vfs);
    strcpy((*vfs) -> actual_path, path);
}