#include "main.h"

void inode_blocks_init(VFS **vfs) {
    (*vfs) -> inode_blocks = (INODE_BLOCK*) malloc(sizeof(INODE_BLOCK));
    (*vfs) -> inode_blocks -> size = 0;

    // Init root directory
    init_root_directory(vfs, (*vfs) -> inode_blocks -> size);
}

void init_root_directory(VFS **vfs, int node_id) {
    // Check if exist free Data block
    if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
        printf("ERROR: Out of memory. Cannot create a file\n");
        return;
    }
    (*vfs) -> inode_blocks -> items[node_id] = (INODE*) malloc(sizeof(INODE));
    (*vfs) -> inode_blocks -> items[node_id] -> nodeid = node_id;
    (*vfs) -> inode_blocks -> items[node_id] -> isDirectory = DIRECTORY;
    (*vfs) -> inode_blocks -> items[node_id] -> references = 1;
    (*vfs) -> inode_blocks -> items[node_id] -> file_size = DIRECTORY;

    // Write to data block
    int32_t cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);
    add_folder_to_structure(vfs, node_id, "root", cluster_id, 1);
    add_folder_to_structure(vfs, node_id, "/", cluster_id, 2);
    set_one_bitmap_on_index(&(*vfs)->bitmap, cluster_id);

    // Add direct links
    int32_t cluster_address;
    cluster_address = (*vfs) -> superblock -> data_start_address + (cluster_id * ONE_CLUSTER_SIZE);
    (*vfs) -> inode_blocks -> items[node_id] -> direct[0] = cluster_address;
    (*vfs) -> inode_blocks -> items[node_id] -> cluster_count = 1;
    (*vfs) -> inode_blocks -> size++;

    fwrite_bitmap(vfs);
    fwrite_inode_block(vfs);
    fwrite_inode_item(vfs, node_id);
    fwrite_data_block(vfs, cluster_id);
}

void add_folder_to_structure(VFS **vfs, int node_id, char *dir_name, int32_t cluster_id,  int directory_size) {
    if (directory_size > MAX_DIR_COUNT) {
        printf("ERROR: Out of memory");
        return;
    }
    
    (*vfs) -> data_blocks -> directory[cluster_id] -> block_id = cluster_id;
    (*vfs) -> data_blocks -> directory[cluster_id] -> size = directory_size;
    // Fill directory item
    (*vfs) -> data_blocks -> directory[cluster_id] -> files[directory_size-1] = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
    (*vfs) -> data_blocks -> directory[cluster_id] -> files[directory_size-1] -> inode = node_id;
    strcpy((*vfs) -> data_blocks -> directory[cluster_id] -> files[directory_size-1] -> item_name, dir_name);
}

void inode_init(VFS **vfs, int node_id, char *name, int isDirectory, int item_size) {
    if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
        printf("ERROR: Out of memory. Cannot create a file\n");
        return;
    }

    (*vfs) -> inode_blocks -> items[node_id] = (INODE*) malloc(sizeof(INODE));
    (*vfs) -> inode_blocks -> items[node_id] -> nodeid = node_id;
    (*vfs) -> inode_blocks -> items[node_id] -> isDirectory = isDirectory;
    (*vfs) -> inode_blocks -> items[node_id] -> references = 1;
    (*vfs) -> inode_blocks -> items[node_id] -> file_size = item_size;

    // Fill data blocks
    int cluster_count = 1;
    if (item_size !=0) {
        cluster_count = item_size / ONE_CLUSTER_SIZE;
        if ((item_size % ONE_CLUSTER_SIZE) != 0) cluster_count++;
    }

    if (isDirectory == 1) {
        fill_data_block_directory(vfs, node_id, name);
    } else {
        fill_data_block_file(vfs, cluster_count);
    }
}

void fill_data_block_directory(VFS **vfs, int node_id, char *name){
    int folder_count;
    folder_count = get_folder_count((*vfs) -> actual_path);

    char *array[10];
    int array_size = get_path_array((*vfs) -> actual_path, array);
    DIR *parent_dir;
    parent_dir = get_directory_by_name(vfs, array[array_size - 1]);
    if (parent_dir == NULL) {
        return;
    } else {
        // Add folder to existing structure
        int directory_size = parent_dir -> size + 1;
        add_folder_to_structure(vfs, node_id, name, parent_dir -> block_id, directory_size);
        // Create new data block
        int32_t cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);
        // Current
        add_folder_to_structure(vfs, node_id, name, cluster_id, 1);
        // Parent
        add_folder_to_structure(vfs, parent_dir -> files[0] -> inode, parent_dir -> files[0] -> item_name, cluster_id, 2);
        set_one_bitmap_on_index(&(*vfs) -> bitmap, cluster_id);
        // Fill direct link
        int32_t cluster_address;
        cluster_address = (*vfs) -> superblock -> data_start_address + (cluster_id * ONE_CLUSTER_SIZE);
        (*vfs) -> inode_blocks -> items[node_id] -> direct[0] = cluster_address;
        (*vfs) -> inode_blocks -> items[node_id] -> cluster_count = 1;
        (*vfs) -> inode_blocks -> size++;
        fwrite_bitmap(vfs);
        fwrite_data_block(vfs, cluster_id);
    }
    fwrite_inode_block(vfs);
    fwrite_inode_item(vfs, node_id);
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

void fwrite_inode_item(VFS **vfs, int node_id) {
    fseek((*vfs) -> FILE, (*vfs) -> superblock -> inode_start_address + sizeof(INODE_BLOCK) + (node_id * sizeof(INODE)), SEEK_SET);
    fwrite((*vfs) -> inode_blocks -> items[node_id], sizeof(INODE), 1, (*vfs) -> FILE);
    fflush((*vfs) -> FILE);
}

void fread_inode_block(VFS **vfs, FILE *file) {
    (*vfs) -> inode_blocks = (INODE_BLOCK*) malloc(sizeof(INODE_BLOCK));
    fseek(file, (*vfs) -> superblock -> inode_start_address, SEEK_SET);
    fread((*vfs) -> inode_blocks, sizeof(INODE_BLOCK), 1, file);

    int i;
    for (i = 0; i < CLUSTER_COUNT; i++) {
        (*vfs) -> inode_blocks -> items[i] = (INODE*) malloc(sizeof(INODE));
        fseek(file, (*vfs) -> superblock -> inode_start_address + sizeof(INODE_BLOCK) + (i * sizeof(INODE)), SEEK_SET);
        fread((*vfs) -> inode_blocks -> items[i], sizeof(INODE), 1, file);
    }
}

INODE *find_directory (VFS **vfs, char *dir_name) {
    char *array[10];
    int array_size = get_path_array((*vfs) -> actual_path, array);
    DIR *dir;
    dir = get_directory_by_name(vfs, array[array_size - 1]);
    if (dir == NULL) return NULL;

    for (int i = 1; i < dir -> size; i++) {
        INODE *inode = get_inode_by_id(vfs, dir -> files[i] -> inode);
        if (inode -> isDirectory == 1) {
            if (strcmp(dir -> files[i] -> item_name, dir_name) == 0) {
                return inode;
            }
        }
    }
    return NULL;
}

INODE *get_inode_by_id (VFS **vfs, int32_t id) {
    for (int i = 0; i < (*vfs) -> inode_blocks -> size; i++) {
        if ((*vfs) -> inode_blocks -> items[i] -> nodeid == id) {
            return (*vfs) -> inode_blocks -> items[i];
        }
    }
    return NULL;
}

int remove_directory (VFS **vfs, INODE *inode) {
    int32_t curr_data_block_id = (inode -> direct[0] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
    DIR *directory = get_block_by_id(vfs, curr_data_block_id);

    // Check if directory is empty
    if (directory -> size > 2){ // 1 -> current; 2 -> parent
        return -1;
    }

    // Get parent folder
    INODE *parent_inode = get_inode_by_id(vfs, directory -> files[1] -> inode);
    int32_t parent_data_block_id = (parent_inode -> direct[0] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
    DIR *parent = get_block_by_id(vfs, parent_data_block_id);

    // Free bitmap
    set_zero_bitmap_on_index(&(*vfs) -> bitmap, curr_data_block_id);
    // Decrease inode map size
    (*vfs) -> inode_blocks -> size--;

    // Remove folder from parent folder
    parent -> size--;

    // Free inode
    free(inode);
    free(directory);

    return 1;
}
