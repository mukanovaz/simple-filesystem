#include "main.h"
#include <stdlib.h>

void inode_blocks_init(VFS **vfs) {
    (*vfs) -> inode_blocks = calloc(1, sizeof(INODE));
    (*vfs) -> inode_blocks -> size = 0;
    (*vfs) -> inode_blocks -> items = calloc((*vfs) -> inode_blocks -> size, sizeof(INODE));

    // Init root directory
    inode_init(vfs, (*vfs) -> inode_blocks -> size, "/", DIRECTORY, DIRECTORY);
}

void inode_init(VFS **vfs, int node_id, char *name, int isDirectory, int item_size) {
    if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
        printf("ERROR: Out of memory. Cannot create a file\n");
        return;
    }

    // TODO: proverit
    (*vfs) -> inode_blocks -> items = realloc((*vfs) -> inode_blocks -> items, ((*vfs) -> inode_blocks -> size + 1) *
            sizeof(INODE));

    (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] = calloc(1, sizeof(INODE));
    (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> nodeid = node_id;
    (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> isDirectory = isDirectory;
    (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> references = 1;
    (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> file_size = item_size;

    // Fill data blocks
    int cluster_count = 1;
    if (item_size !=0) {
        cluster_count = item_size / CLUSTER_SIZE;
        if ((item_size % CLUSTER_SIZE) != 0) cluster_count++;
    }

    if (isDirectory == 1) {
        fill_data_block_directory(vfs, cluster_count, name);
    } else {
        fill_data_block_file(vfs, cluster_count);
    }
}

void fill_data_block_directory(VFS **vfs, int cluster_count, char *name){
    int32_t cluster_id;
    int32_t data_block_address;
    int free_c_exist;
    free_c_exist = check_free_clusters(&(*vfs) -> bitmap, cluster_count);

    if (free_c_exist == 0) {
        printf("ERROR: File is too large");
        return;
    }
    cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);
    data_block_address = ((*vfs) -> superblock -> data_start_address) + (cluster_id * CLUSTER_SIZE);
    (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> direct[0] = data_block_address;
    (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> cluster_count = 1;
    // TODO: create directory_item
    DIR *dir;
    dir_init(&dir, name, cluster_id);

    fwrite_inode_block(vfs);
    fwrite_mft_item(vfs, cluster_id);
//    fwrite_directory(vfs);
    // fwrite_dir

}

void dir_init(DIR **dir, char *name, int32_t node_id) {
//    int size = CLUSTER_SIZE / sizeof(DIR_ITEM);
//    (*dir) -> size = size;
//    (*dir) -> files = calloc((*dir) -> size, sizeof(DIR_ITEM));
//    (*dir) = calloc(1, sizeof(VFS));
//    // TODO: check name len
//    strcpy((*dir) -> item_name, name);
//    (*dir) -> inode = node_id;
//    (*dir) -> files = calloc(size, sizeof(INODE));
}

void fill_data_block_file(VFS **vfs, int cluster_count) {
    int free_c_exist;
    int i = 0;
    int already_setted = 0;
    int start = 0;

    free_c_exist = check_free_clusters(&(*vfs) -> bitmap, cluster_count);

    if (free_c_exist == 0) {
        printf("ERROR: File is too large");
        return;
    }

    while(1) {
        // Get free cluster
        struct the_fragment_temp *temp = find_free_cluster(&(*vfs) -> bitmap, cluster_count - already_setted);
        if (temp -> start_cluster_ID == -1 && temp -> count == 0 && temp -> successful == 0) break;
        already_setted += temp -> count; // 0

        if (already_setted < 5) {
            // Fill direct links
            for (int j = start; j < already_setted; ++j) {
                (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> direct[j] =
                        (*vfs) -> superblock -> data_start_address + 1 + (temp -> start_cluster_ID * CLUSTER_SIZE);
                start = j;
            }
        } else {
            // TODO: fill indirect
        }

        // Check if file is already in blocks
        if (temp -> successful == 1) {
            (*vfs) -> inode_blocks -> items[(*vfs) -> inode_blocks -> size] -> cluster_count = cluster_count;
            break;
        }
        i++;
    }

    // fwrite_inode_block
    // fwrite_inode
}

void fwrite_inode_block(VFS **vfs) {
    fseek((*vfs) -> FILE, (*vfs) -> superblock -> inode_start_address, SEEK_SET);
    fwrite((*vfs) -> inode_blocks, sizeof(INODE_BLOCK), 1, (*vfs) -> FILE);
    fflush((*vfs) -> FILE);
}

void fwrite_mft_item(VFS **vfs, int node_id) {
    fseek((*vfs) -> FILE, (*vfs) -> superblock -> inode_start_address + sizeof(INODE_BLOCK) + (node_id * sizeof(INODE)), SEEK_SET);
    fwrite((*vfs) -> inode_blocks -> items[node_id], sizeof(INODE), 1, (*vfs) -> FILE);
    fflush((*vfs) -> FILE);
}

INODE *get_inode_from_path(VFS *vfs, char *tok) {
//    INODE *item = NULL;
//
//    if (strlen(tok) > 0 && tok[strlen(tok) - 1] == '\n') tok[strlen(tok) - 1] = '\0';
//    char temp_path[strlen(vfs -> actual_path) + strlen(tok)];
//    strcpy(temp_path, vfs -> actual_path);
//
//    if (strlen(tok) > 0 && tok[0] != 47) strcat(temp_path, "/");
//    strcat(temp_path, tok);
//    int folder_ID = find_folder_id(vfs -> inode_blocks, temp_path);
//    item = find_mft_item_by_uid(vfs -> inode_blocks, folder_ID);
//    return item;
}
