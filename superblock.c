#include "main.h"

void superblock_init(SUPERBLOCK **superblock, int32_t disk_size, int32_t cluster_size){
    (*superblock) = calloc(1, sizeof(SUPERBLOCK));
    strcpy((*superblock) -> signature, SIGNATURE);
    strcpy((*superblock) -> volume_descriptor, DESCRIPTOR);

    (*superblock) -> cluster_size = ONE_CLUSTER_SIZE;
    (*superblock) -> cluster_count = disk_size / ONE_CLUSTER_SIZE;
    (*superblock) -> disk_size = (*superblock) -> cluster_count * ONE_CLUSTER_SIZE;

    (*superblock) -> inode_start_address = sizeof(SUPERBLOCK);
    (*superblock) -> bitmap_start_address = (*superblock) -> inode_start_address + (sizeof(INODE) * MAX_INODE_COUNT);
    (*superblock) -> data_start_address = (*superblock) -> bitmap_start_address + sizeof(BITMAP) + ((*superblock) -> cluster_count * sizeof(unsigned char));
}

void fread_superblock(VFS **vfs, FILE *file) {
    (*vfs) -> superblock = calloc(1, sizeof(SUPERBLOCK));
    fread((*vfs) -> superblock, sizeof(SUPERBLOCK), 1, file);
}


void superblock_info(SUPERBLOCK *superblock) {
    printf("Boot record:\n----------------\n");
    printf("Signature: %s\n", superblock -> signature);
    printf("Volume descriptor: %s\n", superblock -> volume_descriptor);
    printf("Disk size: %d B\n", superblock -> disk_size);
    printf("Cluster size: %d B\n", superblock -> cluster_size);
    printf("Cluster count: %d\n", superblock -> cluster_count);
    printf("Inode blocks start address: %d\n", superblock -> inode_start_address);
    printf("BITMAP start address: %d\n", superblock -> bitmap_start_address);
    printf("DATA start address: %d\n", superblock -> data_start_address);
}
