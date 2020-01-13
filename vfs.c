#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include "main.h"

void vfs_init(VFS **vfs, char *filename, size_t disk_size) {
    printf("INFO: Creating virtual file-system based on inode\n");
    if(filename == NULL || strlen(filename) < 1)
    {
        printf("ERROR: Invalid input filename!\n");
        return;
    }

    // Fill filename
    (*vfs) = (VFS*) calloc(1, sizeof(VFS));
    (*vfs) -> filename = calloc(MAX_NAME_LEN, sizeof(char));
    strcpy((*vfs) -> filename, filename);

    // Actual path
    (*vfs) -> actual_path = malloc((strlen(filename) + 1) * sizeof(char));
    strcpy((*vfs) -> actual_path, "/root");

    // Check if file exist
    if(file_exists((*vfs) -> filename))
    {
        FILE *file;
        file = fopen((*vfs) -> filename, "r");

        fread_superblock(vfs, file);
        fread_inode_block(vfs, file);
        fread_bitmap(vfs, file);

        fclose(file);
    } else {
        // Create new file
        FILE *file;
        file =  fopen(filename, "wb");
        if(file == NULL)
        {
            printf("ERROR: Cannot create file %s\n", filename);
            return;
        }
        create_vfs_file(vfs, disk_size, file);
        fclose(file);

        // Create inode blocks
        if (inode_blocks_init(vfs) == -1) {
            return;
        }
    }
}

void create_vfs_file(VFS **vfs, size_t disk_size, FILE *file) {

    if(file == NULL)
    {
        printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
        return;
    }
    fseek(file, 0, SEEK_SET);
    printf("INFO: File .dat not found. Creating new .dat file..\n");

    // Create Superblock
    SUPERBLOCK *superblock;
    superblock_init(&superblock, disk_size, ONE_CLUSTER_SIZE);
    (*vfs) -> superblock = superblock;
    // Write my_superblock
    fseek(file, 0, SEEK_SET);
    fwrite((*vfs) -> superblock, sizeof(SUPERBLOCK), 1, file);

    // Create bitmap
    bitmap_init(vfs, (*vfs) -> superblock -> cluster_count);
    // Write bitmap
    fseek(file, (*vfs) -> superblock -> bitmap_start_address, SEEK_SET);
    fwrite((*vfs) -> bitmap -> data, sizeof(unsigned char), (*vfs) -> bitmap -> length, file);
    fflush(file);
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

    strcpy((*vfs) -> actual_path, path);
}

void systeminfo(VFS **vfs) {
    printf("+-----------------------------------+\n");
    printf("  DISK SIZE: %d\n", (*vfs) -> superblock -> disk_size);
    printf("  SIGNATURE: %s\n", (*vfs) -> superblock -> signature);
    printf("  DESCRIPTOR: %s\n", (*vfs) -> superblock -> volume_descriptor);
    bitmap_info((*vfs) -> bitmap);
    printf("+-----------------------------------+\n");
    printf("              [INODEs]\n");
    printf("  INODES COUNT: %d bytes\n", MAX_INODE_COUNT);
    printf("  INODES: [%d - not used] [%d - used]\n", MAX_INODE_COUNT - (*vfs) -> inode_table -> size, (*vfs) -> inode_table -> size);
    printf("+-----------------------------------+\n");
}