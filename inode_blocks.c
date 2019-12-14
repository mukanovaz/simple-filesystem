#include "main.h"

void inode_blocks_init(VFS **vfs) {
//    (*vfs) -> inode_blocks = calloc(1, sizeof(INODE));
//    (*vfs) -> inode_blocks -> size = 0;
//    (*vfs) -> inode_blocks -> items = calloc((*vfs) -> inode_blocks -> size, sizeof(INODE));
//
//    // Init root directory
//    init_root_directory(vfs, (*vfs) -> inode_blocks -> size);
}

void init_root_directory(VFS **vfs, int node_id) {
//    // Check if exist free Data block
//    if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
//        printf("ERROR: Out of memory. Cannot create a file\n");
//        return;
//    }
//    (*vfs) -> inode_blocks -> items = realloc((*vfs) -> inode_blocks -> items, ((*vfs) -> inode_blocks -> size + 1) *
//                                                                               sizeof(INODE));
//    (*vfs) -> inode_blocks -> items[node_id] = calloc(1, sizeof(INODE));
//    (*vfs) -> inode_blocks -> items[node_id] -> nodeid = node_id;
//    (*vfs) -> inode_blocks -> items[node_id] -> isDirectory = DIRECTORY;
//    (*vfs) -> inode_blocks -> items[node_id] -> references = 1;
//    (*vfs) -> inode_blocks -> items[node_id] -> file_size = DIRECTORY;
//
//    // Write to data block
//    int32_t cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);
//    add_folder_to_structure(vfs, node_id, "/root", cluster_id, 1);
//    set_bitmap_on_index(&(*vfs)-> bitmap, cluster_id);
//
//    // Add direct links
//    int32_t cluster_address;
//    cluster_address = (*vfs) -> superblock -> data_start_address + (cluster_id * ONE_CLUSTER_SIZE);
//    (*vfs) -> inode_blocks -> items[node_id] -> direct[0] = cluster_address;
//    (*vfs) -> inode_blocks -> items[node_id] -> cluster_count = 1;
//    (*vfs) -> inode_blocks -> size++;
}

void add_folder_to_structure(VFS **vfs, int node_id, char *dir_name, int32_t cluster_id,  int directory_size) {
    if (directory_size > MAX_DIR_COUNT) {
        printf("ERROR: Out of memory");
        return;
    }
    (*vfs) -> data_blocks -> directory[cluster_id] = malloc(sizeof(DIR_ITEM));
    (*vfs) -> data_blocks -> directory[cluster_id] -> block_id = cluster_id;
    (*vfs) -> data_blocks -> directory[cluster_id] -> size = directory_size;
    (*vfs) -> data_blocks -> directory[cluster_id] -> files = calloc((*vfs) -> data_blocks -> directory[cluster_id] -> size, sizeof(DIR_ITEM));
    // Fill directory item
    (*vfs) -> data_blocks -> directory[cluster_id] -> files[0] = malloc(sizeof(DIR_ITEM));
    (*vfs) -> data_blocks -> directory[cluster_id] -> files[0] -> inode = node_id;
    strcpy((*vfs) -> data_blocks -> directory[cluster_id] -> files[0] -> item_name, dir_name);
}

void inode_init(VFS **vfs, int node_id, char *name, int isDirectory, int item_size) {
//    if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
//        printf("ERROR: Out of memory. Cannot create a file\n");
//        return;
//    }
//
//    (*vfs) -> inode_blocks -> items = realloc((*vfs) -> inode_blocks -> items, ((*vfs) -> inode_blocks -> size + 1) *
//            sizeof(INODE));
//
//    (*vfs) -> inode_blocks -> items[node_id] = calloc(1, sizeof(INODE));
//    (*vfs) -> inode_blocks -> items[node_id] -> nodeid = node_id;
//    (*vfs) -> inode_blocks -> items[node_id] -> isDirectory = isDirectory;
//    (*vfs) -> inode_blocks -> items[node_id] -> references = 1;
//    (*vfs) -> inode_blocks -> items[node_id] -> file_size = item_size;
//
//    // Fill data blocks
//    int cluster_count = 1;
//    if (item_size !=0) {
//        cluster_count = item_size / ONE_CLUSTER_SIZE;
//        if ((item_size % ONE_CLUSTER_SIZE) != 0) cluster_count++;
//    }
//
//    if (isDirectory == 1) {
//        fill_data_block_directory(vfs, node_id, name);
//    } else {
//        fill_data_block_file(vfs, cluster_count);
//    }
}

void fill_data_block_directory(VFS **vfs, int node_id, char *name){
    int folder_count;
    folder_count = get_folder_count((*vfs) -> actual_path);

    if (folder_count == 1) {        // ROOT
        // Add to root structure new folder
        int directory_size =  (*vfs) -> data_blocks -> directory[0] -> size + 1;
        add_folder_to_structure(vfs, node_id, "/root", 0, directory_size);
        // Create new data block
        int32_t cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);
        add_folder_to_structure(vfs, node_id, name, cluster_id, 1);
        // Fill direct link
        int32_t cluster_address;
        cluster_address = (*vfs) -> superblock -> data_start_address + (cluster_id * ONE_CLUSTER_SIZE);
        (*vfs) -> inode_blocks -> items[node_id] -> direct[0] = cluster_address;
        (*vfs) -> inode_blocks -> items[node_id] -> cluster_count = 1;
        (*vfs) -> inode_blocks -> size++;
    } else if (folder_count > 1) {  // OTHER
        // Find last path node
        printf("INFO: NOT YET");
    } else {
        printf("ERROR: Invalid path");
        return;
    }
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
                        (*vfs) -> superblock -> data_start_address + 1 + (temp -> start_cluster_ID * ONE_CLUSTER_SIZE);
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

INODE *get_root_inode (INODE_BLOCK *inodes) {
    return inodes -> items[0];
}

INODE *find_directory (VFS **vfs, char *dir_name) {
    for (int i = 0; i < (*vfs) -> inode_blocks -> size; i++) {
        if ((*vfs) -> inode_blocks -> items[i] -> isDirectory == 1) {
            // Find directory in datablocks
            int32_t cluster_id = ((*vfs) -> inode_blocks -> items[i] -> direct[0] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
            int exist = directory_exist(vfs, cluster_id, dir_name);
            if (exist == 1) {
                return (*vfs) -> inode_blocks -> items[i];
            }
        }
    }
    return NULL;
}
