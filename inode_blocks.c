#include "main.h"

int inode_blocks_init(VFS **vfs) {
    (*vfs) -> inode_blocks = (INODE_BLOCK*) malloc(sizeof(INODE_BLOCK));
    (*vfs) -> inode_blocks -> size = 0;

    // Init root directory
    if(init_root_directory(vfs, (*vfs) -> inode_blocks -> size) == -1) {
        free((*vfs) -> inode_blocks);
        return -1;
    }
    return 1;
}

int init_cluster_directory (VFS **vfs, int cluster_id) {
    // Get cluster address
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
        return -1;
    }
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);
    int32_t data_block_addr = (*vfs) -> superblock -> data_start_address + (ONE_CLUSTER_SIZE * cluster_id);
    for (int i = 0; i < max_dir_count; i++) {
        int32_t new_dir_item_addr = data_block_addr + (sizeof(DIR_ITEM) + i);
        DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
        item -> inode = ID_ITEM_FREE;

        fseek(file, new_dir_item_addr, SEEK_SET);
        fwrite(item, sizeof(DIR_ITEM), 1,file);
        free(item);
    }

    return -1;
}


int init_root_directory(VFS **vfs, int node_id) {
    // Check if exist free Data block
    if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
        printf("ERROR: Out of memory. Cannot create a file\n");
        return -1;
    }
    (*vfs) -> inode_blocks -> items[node_id] = (INODE*) malloc(sizeof(INODE));
    (*vfs) -> inode_blocks -> items[node_id] -> nodeid = node_id;
    (*vfs) -> inode_blocks -> items[node_id] -> isDirectory = DIRECTORY;
    (*vfs) -> inode_blocks -> items[node_id] -> references = 1;
    (*vfs) -> inode_blocks -> items[node_id] -> file_size = DIRECTORY;

    // Write to data block
    int32_t cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);
    init_cluster_directory(vfs, cluster_id);
    int ok1 = add_folder_to_structure(vfs, node_id, "root", cluster_id, 0);
    if (ok1 == -1) {
        free((*vfs) -> inode_blocks -> items[node_id]);
        return -1;
    }
    int ok2 = add_folder_to_structure(vfs, node_id, "/", cluster_id, 1);
    if (ok2 == -1) {
        free((*vfs) -> inode_blocks -> items[node_id]);
        return -1;
    }

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
    return 1;
}

// TODO: obrabotat return
int add_folder_to_structure(VFS **vfs, int node_id, char *dir_name, int32_t cluster_id,  int dir_item_id) {
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if (dir_item_id > max_dir_count) {
        printf("ERROR: Out of memory");
        return -1;
    }

    if(file == NULL)
    {
        printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
        return -1;
    }

    DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
    item -> inode = node_id;
    strcpy(item -> item_name, dir_name);

    // Write into .dat
    int32_t data_block_addr = (*vfs) -> superblock -> data_start_address + (ONE_CLUSTER_SIZE * cluster_id);
    int32_t new_dir_item_addr = data_block_addr + (sizeof(DIR_ITEM) * dir_item_id);

    fseek(file, new_dir_item_addr, SEEK_SET);
    fwrite(item, sizeof(DIR_ITEM), 1, file);
    fclose(file);

    free(item);
    return 1;
}

int inode_init(VFS **vfs, int node_id, char *name, int isDirectory, int item_size, int32_t parent_node_id) {
    if (bitmap_contains_free_cluster((*vfs) -> bitmap) == 1) {
        printf("ERROR: Out of memory. Cannot create a file\n");
        return -1;
    }

    (*vfs) -> inode_blocks -> items[node_id] = (INODE*) malloc(sizeof(INODE));
    (*vfs) -> inode_blocks -> items[node_id] -> nodeid = node_id;
    (*vfs) -> inode_blocks -> items[node_id] -> isDirectory = isDirectory;
    (*vfs) -> inode_blocks -> items[node_id] -> references = 1;
    (*vfs) -> inode_blocks -> items[node_id] -> file_size = item_size;
    (*vfs) -> inode_blocks -> size++;

    // Fill data blocks
    int cluster_count = 1;
    if (item_size !=0) {
        cluster_count = item_size / ONE_CLUSTER_SIZE;
        if ((item_size % ONE_CLUSTER_SIZE) != 0) cluster_count++;
    }

    if (isDirectory == 1) {
        if (fill_data_block_directory(vfs, node_id, name, parent_node_id) == -1) {
            free((*vfs) -> inode_blocks -> items[node_id]);
            (*vfs) -> inode_blocks -> size--;
            return -1;
        }
    } else {
        // TODO: (VFS **vfs, char *name, int cluster_count, int node_id, int32_t parent_node_id)
        if (fill_data_block_file(vfs, name, cluster_count, node_id, parent_node_id) != 1) {
            free((*vfs) -> inode_blocks -> items[node_id]);
            (*vfs) -> inode_blocks -> size--;
            return -1;
        }
    }

    return node_id;
}

int fill_data_block_directory(VFS **vfs, int node_id, char *new_folder_name, int32_t parent_node_id){
    char* str = malloc(strlen(new_folder_name));
    strcpy(str, new_folder_name);
    removeChar(str, '/');

    // Get actual path parent_inode -> parent parent_inode
    INODE *parent_inode = get_inode_by_id(vfs, parent_node_id);

    if (parent_inode == NULL) {
        printf("ERROR: cannot find actual path parent_inode\n");
        free(str);
        return -1;
    }
    // Create new directory cluster
    int32_t cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);
    init_cluster_directory(vfs, cluster_id);

    // Fill dir_item [0] -> actual folder
    if (add_folder_to_structure(vfs, node_id, new_folder_name, cluster_id, 0) == -1) {
        free(str);
        return -1;
    }
    // Fill dir_item [1] -> parent folder
    if (add_folder_to_structure(vfs, parent_inode -> nodeid, str, cluster_id, 1) == -1) {
        free(str);
        return -1;
    }
    // Update parent structure
    int index = get_free_dir_item_id(vfs, parent_inode);
    int parent_cluster_id = (parent_inode -> direct[0] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
    if (add_folder_to_structure(vfs, node_id, new_folder_name, parent_cluster_id, index) == -1) {
        free(str);
        return -1;
    }
    // Update bitmap
    set_one_bitmap_on_index(&(*vfs) -> bitmap, cluster_id);
    // Fill direct link
    int32_t cluster_address;
    cluster_address = (*vfs) -> superblock -> data_start_address + (cluster_id * ONE_CLUSTER_SIZE);
    (*vfs) -> inode_blocks -> items[node_id] -> direct[0] = cluster_address;
    (*vfs) -> inode_blocks -> items[node_id] -> cluster_count = 1;
    // Write data to file
    fwrite_bitmap(vfs);
    fwrite_inode_block(vfs);
    fwrite_inode_item(vfs, node_id);
    free(str);
    return 1;
 }

int fill_data_block_file(VFS **vfs, char *name, int cluster_count, int node_id, int32_t parent_node_id) {
    int free_c_exist;
    int i = 0;

    free_c_exist = check_free_clusters(&(*vfs) -> bitmap, cluster_count);
    if (free_c_exist == 0) {
        printf("ERROR: File is too large");
        return -1;
    }

    // Add file to folder
    INODE *parent_inode = get_inode_by_id(vfs, parent_node_id);
    if (parent_inode == NULL) {
        printf("ERROR: cannot find actual path parent_inode\n");
        return -1;
    }
    int index = get_free_dir_item_id(vfs, parent_inode);
    int parent_cluster_id = (parent_inode -> direct[0] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
    if (add_folder_to_structure(vfs, node_id, name, parent_cluster_id, index) == -1) {
        return -1;
    }

    // Fill direct and indirect links
    for (i = 0; i < cluster_count; i++) {
        int32_t cluster_id = get_one_free_cluster(&(*vfs) -> bitmap);

        // Direct links
        if (i <= MAX_DIRECT_LINKS) {
            int32_t cluster_address = (*vfs) -> superblock -> data_start_address + (cluster_id * ONE_CLUSTER_SIZE);
            (*vfs) -> inode_blocks -> items[node_id] -> direct[i] = cluster_address;
        } else {
            // Indirect links
        }
        (*vfs) -> inode_blocks -> items[node_id] -> cluster_count++;
        set_one_bitmap_on_index(&(*vfs) -> bitmap, cluster_id);
    }

    // Write to .dat file
    fwrite_bitmap(vfs);
    fwrite_inode_block(vfs);
    fwrite_inode_item(vfs, node_id);

    return 1;
}

void fwrite_inode_block(VFS **vfs) {
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
        return;
    }
    fseek(file, (*vfs) -> superblock -> inode_start_address, SEEK_SET);
    fwrite((*vfs) -> inode_blocks, sizeof(INODE_BLOCK), 1, file);
    fflush(file);
    fclose(file);
}

void fwrite_inode_item(VFS **vfs, int node_id) {
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
        return;
    }
    fseek(file, (*vfs) -> superblock -> inode_start_address + sizeof(INODE_BLOCK) + (node_id * sizeof(INODE)), SEEK_SET);
    fwrite((*vfs) -> inode_blocks -> items[node_id], sizeof(INODE), 1, file);
    fflush(file);
    fclose(file);
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


int remove_file_from_directory(VFS **vfs, int32_t parent_inode_id, int32_t inode_id) {
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
        return -1;
    }

    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);
    int32_t block_start_addr = (*vfs) -> inode_blocks -> items[parent_inode_id] -> direct[0];

    for (int i = 0; i < max_dir_count; i++) {
        int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * i);
        DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
        fseek(file, dir_item_addr, SEEK_SET);
        fread(item, sizeof(DIR_ITEM), 1, file);

        if (item -> inode == inode_id) {
            // Change inode
            item -> inode = ID_ITEM_FREE;
            strcpy(item -> item_name, "");
            // Save
            fseek(file, dir_item_addr, SEEK_SET);
            fwrite(item, sizeof(DIR_ITEM), 1, file);
            free(item);
            fclose(file);
            return 1;
        }
        free(item);
    }
    fclose(file);
    return -1;
}

int get_free_dir_item_id (VFS **vfs, INODE *inode) {
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);
    int32_t block_start_addr = inode -> direct[0];

    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
        return -1;
    }
    for (int i = 0; i < max_dir_count; i++) {
        int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * i);
        DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
        fseek(file, dir_item_addr, SEEK_SET);
        fread(item, sizeof(DIR_ITEM), 1, file);

        if (item -> inode == ID_ITEM_FREE) {
            free(item);
            fclose(file);
            return i;
        }
        free(item);
    }
    fclose(file);
    return -1;
}

INODE *find_inode_by_name (VFS **vfs, char *name) {
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);

    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return NULL;
    }
    for (int j = 0; j < (*vfs) -> inode_blocks -> size; j++) {
        if ((*vfs) -> inode_blocks -> items[j] -> isDirectory) {
            int32_t block_start_addr = (*vfs) -> inode_blocks -> items[j] -> direct[0];
            for (int i = 0; i < max_dir_count; i++) {
                int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * i); // First directory item
                // Compare name with first dir item
                DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
                fseek(file, dir_item_addr, SEEK_SET);
                fread(item, sizeof(DIR_ITEM), 1, file);

                if (strcmp(item -> item_name, name) == 0) {
                    int id = item -> inode;
                    free(item);
                    fclose(file);
                    return (*vfs) -> inode_blocks -> items[id];
                }
                free(item);
            }
        }
    }
    fclose(file);
    return NULL;
}

INODE *directory_exist (VFS **vfs, char *actual_folder_name, char *new_folder_name) {
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);

    char* str = malloc(strlen(actual_folder_name));
    strcpy(str, actual_folder_name);
    removeChar(str, '/');

    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
         return NULL;
    }

    INODE *inode = find_inode_by_name(vfs, str);
    // Try to find same directory name
    if (inode != NULL) {
        int32_t block_start_addr = inode -> direct[0];

        for (int i = 0; i < max_dir_count; i++) {
            int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * i);
            // Compare name with first dir item
            DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
            fseek(file, dir_item_addr, SEEK_SET);
            fread(item, sizeof(DIR_ITEM), 1, file);

            if (strcmp(item -> item_name, new_folder_name) == 0) {
                free(item);
                free(str);
                fclose(file);
                return inode;
            }
            free(item);
        }
    } else {
        fclose(file);
        free(str);
        return NULL;
    }

    fclose(file);
    return NULL;
}

INODE *find_directory (VFS **vfs, char *dir_name) {
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);

    char * token, * last;
    last = token = strtok(dir_name, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);

    char* str = malloc(strlen(last));
    strcpy(str, last);
    removeChar(str, '/');

    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return NULL;
    }
    // Get actual path inode
    INODE *inode = find_inode_by_name(vfs, str);
    // Try to find same directory name
    if (inode != NULL) {
        if (inode -> isDirectory) {
            int32_t block_start_addr = inode -> direct[0];

            for (int i = 0; i < max_dir_count; i++) {
                int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * i);
                // Compare name with first dir item
                DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
                fseek(file, dir_item_addr, SEEK_SET);
                fread(item, sizeof(DIR_ITEM), 1, file);

                if (strcmp(item -> item_name, str) == 0) {
                    free(item);
                    fclose(file);
                    free(str);
                    return inode;
                }
                free(item);
            }
        }
    } else {
//        printf("ERROR: cannot find actual path inode\n");
        fclose(file);
        free(str);
        return NULL;
    }
    fclose(file);
    free(str);
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
int remove_file_from_fs (VFS **vfs, INODE *inode) {
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);

    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return -1;
    }

    INODE *parent_inode;
    // Delete file from directory
    for (int j = 0; j < (*vfs) -> inode_blocks -> size; j++) {
        if ((*vfs) -> inode_blocks -> items[j] -> isDirectory) {
            int32_t block_start_addr = (*vfs) -> inode_blocks -> items[j] -> direct[0];
            for (int i = 0; i < max_dir_count; i++) {
                int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * i); // First directory item
                // Compare name with first dir item
                DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
                fseek(file, dir_item_addr, SEEK_SET);
                fread(item, sizeof(DIR_ITEM), 1, file);

                if (item -> inode == inode -> nodeid) {
                    parent_inode = (*vfs) -> inode_blocks -> items[j];
                    free(item);
                    break;
                }
                free(item);
            }
        }
    }
    fclose(file);

    if (parent_inode == NULL) {
        free(parent_inode);
        return -1;
    }

    if (remove_file_from_directory(vfs, parent_inode -> nodeid, inode -> nodeid) == -1) {
        printf("ERROR: cannot remove folder from directory");
        free(parent_inode);
        return -1;
    }


    // Free data blocks
    for (int k = 0; k < inode -> cluster_count; k++) {
        // TODO: indirect
        int cluster_id = (inode -> direct[k] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
        // Free bitmap
        set_zero_bitmap_on_index(&(*vfs) -> bitmap, cluster_id);
    }

    // Decrease inode map size
    (*vfs) -> inode_blocks -> size--;

    fwrite_bitmap(vfs);
    fwrite_inode_block(vfs);
    fwrite_inode_item(vfs, inode -> nodeid);
    return 1;
}

int remove_directory (VFS **vfs, INODE *inode) {
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return -1;
    }
    // Check if directory is free dir_item[0] == -1
    int32_t block_start_addr = inode -> direct[0];
    int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * 2); // Get first directory item

    DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
    fseek(file, dir_item_addr, SEEK_SET);
    fread(item, sizeof(DIR_ITEM), 1, file);

    // Check if directory is free
    if (item -> inode != ID_ITEM_FREE) {
        printf("ERROR: directory is not free");
        fclose(file);
        free(item);
        return -2;
    }

    // Free data block
    int cluster_id = (inode -> direct[0] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;

    // Get parent folder
    dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * 1); // Get parent directory item
    DIR_ITEM *parent_item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
    fseek(file, dir_item_addr, SEEK_SET);
    fread(parent_item, sizeof(DIR_ITEM), 1, file);
    fclose(file);

    // Find and free inode from parent folder
    if (remove_file_from_directory(vfs, parent_item->inode, inode->nodeid) == -1) {
        printf("ERROR: cannot remove folder from directory");
        return -1;
    }

    // Free bitmap
    set_zero_bitmap_on_index(&(*vfs) -> bitmap, cluster_id);
    // Decrease inode map size
    (*vfs) -> inode_blocks -> size--;

    // Free inode
    free(inode);
    free(parent_item);

    fwrite_bitmap(vfs);
    fwrite_inode_block(vfs);
    fwrite_inode_item(vfs, inode -> nodeid);
    return 1;
}


int make_file_in_inodes(VFS **vfs, char *source_name, char *dest_name, INODE *dest_inode) {
    int32_t  file_size;

    FILE *source_file = fopen(source_name, "rb");
    if (source_file == NULL) {
        printf("ERROR: cannot open a file %s\n", source_name);
        return -1;
    }

    fseek(source_file, 0, SEEK_END);
    file_size = ftell(source_file);
    fseek(source_file, 0, SEEK_SET);

    // Check if file already exist
    INODE *inode = directory_exist(vfs, (*vfs) -> actual_path, dest_name);
    if (inode != NULL) {
        printf("ERROR: file %s already exist", dest_name);
        return -1;
    }

    // int inode_init(VFS **vfs, int node_id, char *name, int isDirectory, int item_size, int32_t parent_node_id)
    int new_inode = inode_init(vfs, (*vfs) -> inode_blocks -> size, dest_name, MY_FILE, file_size, dest_inode -> nodeid);
    if (new_inode == -1) {
        return -1;
    }

    printf("INFO: file %s created", dest_name);

    // Fill clusters
    int cluster_count = (*vfs) -> inode_blocks -> items[new_inode] -> cluster_count;
    char buffer[cluster_count][ONE_CLUSTER_SIZE];

    for (int i = 0; i < cluster_count; i++) {
        fseek(source_file, i * ONE_CLUSTER_SIZE, SEEK_SET);
        fread(buffer[i], ONE_CLUSTER_SIZE, 1, source_file);
        if (i == (cluster_count - 1)) buffer[i][strlen(buffer[i]) - 1] = '\0';
    }
    fclose(source_file);

    // Fill my FS
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return -1;
    }

    for (int i = 0; i < cluster_count; i++) {
        // TODO: add indirect
        int cluster_id = ((*vfs) -> inode_blocks -> items[new_inode] -> direct[0] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
        int32_t data_block_addr = (*vfs) -> superblock -> data_start_address + (ONE_CLUSTER_SIZE * cluster_id);
        fseek(file, data_block_addr, SEEK_SET);
        fwrite(buffer[i], ONE_CLUSTER_SIZE, 1, file);
        fflush(file);
    }

    fclose(file);
    return 1;
}
