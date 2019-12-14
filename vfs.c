#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include "main.h"

void vfs_init(VFS **vfs, char *filename, size_t disk_size) {
    printf("INFO: Creating virtual file-system based on inode");
    // Fill filename
    (*vfs) = calloc(1, sizeof(VFS));
    (*vfs) -> filename = calloc(MAX_NAME_LEN, sizeof(char));

    // TODO: peredelat'
    strcpy((*vfs) -> filename, filename);

    // Actual path
    (*vfs) -> actual_path = calloc(PATH_MAX, sizeof(char));

    // Open .dat file
    FILE *vfs_file = fopen(filename, "r+");
    if (vfs_file == NULL) {
        // Create new file
        printf("INFO: File .dat not found. Creating new .dat file..\n");
        (*vfs) -> FILE = fopen((*vfs) -> filename, "w+");

        // Create Superblock
        SUPERBLOCK *superblock;
        superblock_init(&superblock, disk_size, CLUSTER_SIZE);
        (*vfs) -> superblock = superblock;

        // Create bitmap
        BITMAP *bitmap;
        bitmap_init(&bitmap, (*vfs) -> superblock -> cluster_count);
        (*vfs) -> bitmap = bitmap;

        // Create inode blocks
        inode_blocks_init(vfs);

        create_vfs_file(vfs);

    } else {
        // TODO: dodelat
    }

}

void create_vfs_file(VFS **vfs) {
    (*vfs) -> FILE = fopen((*vfs) -> filename, "r+");
    if ((*vfs) -> FILE == NULL) {
        printf("File %s not found\n", (*vfs) -> filename);
        return;
    }
    else {
        fwrite((*vfs) -> superblock, sizeof(SUPERBLOCK), 1, (*vfs) -> FILE);
        fflush((*vfs) -> FILE);

        fseek((*vfs) -> FILE, (*vfs) -> superblock -> bitmap_start_address, SEEK_SET);
        fwrite((*vfs) -> bitmap -> data, sizeof(unsigned char), (*vfs) -> bitmap -> length, (*vfs) -> FILE);
        fflush((*vfs) -> FILE);

        fseek((*vfs) -> FILE, (*vfs) -> superblock -> data_start_address, SEEK_SET);
    }
}