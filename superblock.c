#include "main.h"

void superblock_init(SUPERBLOCK **superblock, int32_t disk_size, int32_t cluster_size){
    (*superblock) = calloc(1, sizeof(SUPERBLOCK));
    strcpy((*superblock) -> signature, SIGNATURE);
    strcpy((*superblock) -> volume_descriptor, DESCRIPTOR);

    (*superblock) -> disk_size = disk_size;
    (*superblock) -> cluster_size = cluster_size;
    (*superblock) -> cluster_count = disk_size / cluster_size;
    (*superblock) -> inode_start_address = sizeof(SUPERBLOCK);
    (*superblock) -> bitmap_start_address = (*superblock) -> inode_start_address + sizeof(INODE) + ((*superblock) -> disk_size * INODES_BLOCK_SIZE_RATIO);
    (*superblock) -> data_start_address = (*superblock) -> bitmap_start_address + sizeof(BITMAP) + ((*superblock) -> cluster_count * sizeof(unsigned char));
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
