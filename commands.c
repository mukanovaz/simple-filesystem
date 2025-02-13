#include <values.h>
#include "main.h"


void actual_directory(VFS *vfs) {
    char path[strlen(vfs -> actual_path)];
    if (strlen(vfs -> actual_path) > 0) strcpy(path, vfs -> actual_path);
    else strcpy(path, "/");
    printf("%s", path);
}

void make_directory(VFS **vfs, char *tok) {
    char *ptr = strtok(NULL, SPLITTER);
    // Delete \n
    if (strlen(ptr) > 0 && tok[strlen(ptr) - 1] == '\n') tok[strlen(ptr) - 1] = '\0';

    char *full_path = malloc(strlen((*vfs) -> actual_path) + strlen(ptr) + sizeof(char));
    strcpy(full_path, (*vfs) -> actual_path);
    strcat(full_path, "/");
    strcat(full_path, ptr);

    // Posledni token
    char * token, * last;
    last = token = strtok(ptr, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);

    // Cesta bez koncove slozky
    char *path = malloc(strlen(full_path));
    strcpy(path, full_path);
    path[strlen(full_path) - strlen(last)] = '\0';

    // Parent folder name
    char * t, * parent;
    parent = t = strtok(path, "/");
    for (;(t = strtok(NULL, "/")) != NULL; parent = t);

    // Find directory in path
    INODE *inode = directory_exist(vfs, path, last);
    INODE *parent_inode = find_inode_by_name(vfs, parent);
    if (parent_inode == NULL) return;

    if (inode == NULL) {
        if(inode_init(vfs, (*vfs) -> inode_table -> size, last, DIRECTORY, DIRECTORY, parent_inode -> nodeid) == -1) {
            printf("ERROR: cannot create directory '%s'", last);
            free(full_path);
            free(path);
            return;
        }
        printf("OK");
    } else {
        printf("ERROR: A folder with name '%s' already exists\n", last);
        free(full_path);
        free(path);
        return;
    }
}

void list_files_and_directories(VFS **vfs, char *tok){
    char* str = malloc(strlen((*vfs) -> actual_path));
    strcpy(str, (*vfs) -> actual_path);

    // Posledni token
    char * token, * last;
    last = token = strtok(str, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);

    INODE *inode = find_inode_by_name(vfs, last);
    if (inode == NULL) {
        printf("ERROR: cannot find actual path inode\n");
        free(str);
        return;
    }

    FILE *file = fopen((*vfs) -> filename, "r+b");
    int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);
    int32_t block_start_addr = inode -> direct[0];

    for (int i = 0; i < max_dir_count; i++) {
        if (i == 0) {
            printf("\t+ .\n");
            continue;
        }
        if (i == 1) {
            printf("\t+ ..\n");
            continue;
        }
        int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * i);
        // Compare name with first dir item
        DIR_ITEM *item = (DIR_ITEM*) malloc(sizeof(DIR_ITEM));
        fseek(file, dir_item_addr, SEEK_SET);
        fread(item, sizeof(DIR_ITEM), 1, file);
        if (item -> inode == ID_ITEM_FREE) {
            free(item);
            continue;
        }

        INODE *tmp = get_inode_by_id(vfs, item -> inode);
        if (tmp == NULL) continue;
        if (tmp -> isDirectory == 1) {
            printf("\t+ ");
        } else {
            printf("\t- ");
        }
        printf("%s\n", item -> item_name);
        free(item);
    }
    free(str);
    fclose(file);
}

void change_directory(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        return;
    }

    // Back
    if (strncmp(dir_name, "..", 2) == 0) {
        if (strcmp((*vfs) -> actual_path , "/root") == 0) {
            return;
        }

        if (strlen((*vfs) ->  actual_path) > 1) {
            go_to_parent_folder(vfs);
        }
    }
    else {
        // Delete \n
        if (strlen(dir_name) > 0 && tok[strlen(dir_name) - 1] == '\n') tok[strlen(dir_name) - 1] = '\0';
        unsigned long len = strlen((*vfs) -> actual_path) + strlen(dir_name);
        char temp_path[len + 1];
        strcpy(temp_path, (*vfs) -> actual_path);

        if (dir_name[0] != 47) strcat(temp_path, "/");
        strcat(temp_path, dir_name);

        // Find this address
        INODE *inode = find_directory(vfs, dir_name);
        if (inode == NULL) {
            printf("ERROR: %s ", FILE_IS_DIR);
            return;
        } else {
            if (!inode -> isDirectory) {
                printf("ERROR: %s ", FILE_IS_DIR);
                return;
            }

            strcpy((*vfs) -> actual_path, temp_path);
        }

    }
}

void remove_empty_directory(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        printf("ERROR: Invalid directory name\n");
        return;
    }
    // Find directory in path
    INODE *inode = find_directory(vfs, dir_name);
    if (inode == NULL) {
        printf("ERROR: %s", DIR_NOT_FOUND);
        return;
    } else {
        if (!inode -> isDirectory) {
            printf("ERROR: %s", FILE_IS_DIR);
            return;
        }
        int ok = remove_directory(vfs, inode);
        if (ok == -1) {
            return;
        } else if (ok == -2) {
            return;
        } else {
            printf("OK");
        }
    }
}

void inode_info(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        printf("ERROR: Invalid directory name\n");
        return;
    }
    if (strlen(dir_name) > 0 && dir_name[strlen(dir_name) - 1] == '\n') dir_name[strlen(dir_name) - 1] = '\0';

    INODE *inode = find_inode_by_name(vfs, dir_name);
    if (inode == NULL) {
        printf("ERROR: %s", DIR_NOT_FOUND);
        return;
    }

    printf("+-----------------------------------------------------------------------------------------+\n");
    printf("| INODE ID: %d\n", inode -> nodeid);
    if (inode -> isDirectory) {
        // Print name
        printf("| INODE: Directory\n");
        FILE *file = fopen((*vfs) -> filename, "r+b");
        int32_t block_start_addr = inode -> direct[0];

        int32_t dir_item_addr = block_start_addr + (sizeof(DIR_ITEM) * 0);
        // Compare name with first dir item
        DIR_ITEM *item = (DIR_ITEM *) malloc(sizeof(DIR_ITEM));
        fseek(file, dir_item_addr, SEEK_SET);
        fread(item, sizeof(DIR_ITEM), 1, file);

        printf("| INODE NAME: %s\n", item -> item_name);

        free(item);
        fclose(file);
    }
    else
        printf("| INODE: File\n");

    printf("| INODE size: %d\n", inode -> file_size);

    int32_t *address = get_address(vfs, inode);
    for (int i = 0; i < inode -> cluster_count; i++) {
        if (i < MAX_DIRECT_LINKS)
            printf("| INODE direct[%d]: %d\n", i, address[i]);
        else {
            printf("| INODE indirect1[%d]: %d\n", i, address[i]);
        }
    }

    printf("+-----------------------------------------------------------------------------------------+\n");
}

void incp(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        printf("ERROR: Invalid source path\n");
        return;
    }

    char *dest = strtok(NULL, SPLITTER);
    if (dest == NULL || strlen(dest) <= 1) {
        printf("ERROR: Invalid destination path\n");
        return;
    }
    if (strlen(dest) > 0 && dest[strlen(dest) - 1] == '\n') dest[strlen(dest) - 1] = '\0';

    // Check if source exist
    FILE *file_src = fopen(dir_name, "rb");
    if (file_src == NULL) {
        printf("ERROR: cannot open a file %s\n", dir_name);
        return;
    }
    fclose(file_src);

    char *full_path = malloc(strlen(dest));
    strcpy(full_path, (*vfs) -> actual_path);
    strcat(full_path, "/");
    strcat(full_path, dest);

    char * token, * last;
    last = token = strtok(dest, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);

    char *path = malloc(strlen(full_path));
    strcpy(path, full_path);
    path[strlen(full_path) - strlen(last)] = '\0';

    INODE *destination = find_directory(vfs, path);
    if (destination == NULL) {
        free(full_path);
        free(path);
        return;
    }

    if (make_file_in_inodes(vfs, dir_name, last, destination) == -1) {
        printf("ERROR: cannot import file.");
        free(full_path);
        free(path);
        return;
    }
    free(full_path);
    free(path);
    printf("INFO: OK");
}


void concatenate(VFS **vfs, char *tok) {
    char buffer[ONE_CLUSTER_SIZE];
    int i;
    char *dir_name = strtok(NULL, SPLITTER);

    if (dir_name == NULL || strlen(dir_name) <= 1) {
        printf("ERROR: Invalid directory name\n");
        return;
    }
    // Check if node exist
    INODE *inode = find_inode_by_name(vfs, dir_name);
    if (inode == NULL) {
        printf("ERROR: %s", DIR_NOT_FOUND);
        return;
    }
    int actual_size = inode -> file_size;

    // Write file to console
    FILE *file;
    file = fopen((*vfs) -> filename, "rb");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return;
    }
    printf("\n");


    int32_t *address = get_address(vfs, inode);
    for (i = 0; i < inode -> cluster_count; i++) {
        fseek(file, address[i], SEEK_SET);
        if (actual_size >= ONE_CLUSTER_SIZE) {
            fread(buffer, ONE_CLUSTER_SIZE, 1, file);
            printf("%s", buffer);
            actual_size -= ONE_CLUSTER_SIZE;
        }
        else {
            fread(buffer, actual_size, 1, file);
            int k;
            for (k = 0; k < actual_size; k++) {
                printf("%c", buffer[k]);
            }
            break;
        }
    }
    free(address);
    printf("\n");
    fclose(file);
}

void outcp(VFS **vfs, char *tok) {
    char *source = strtok(NULL, SPLITTER);
    if (source == NULL || strlen(source) <= 1) {
        printf("ERROR: Invalid source path\n");
        return;
    }

    char *dest = strtok(NULL, SPLITTER);
    if (dest == NULL || strlen(dest) <= 1) {
        printf("ERROR: Invalid destination path\n");
        return;
    }

    INODE *inode = find_inode_by_name(vfs, source);
    if (inode == NULL) {
        printf("ERROR: %s", DIR_NOT_FOUND);
        return;
    }

    if (inode -> isDirectory) {
        printf("ERROR: cannot copy directory\n");
        return;
    }

    // Write file to file
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return;
    }

    int cluster_count = inode -> cluster_count;
    char buffer[cluster_count][ONE_CLUSTER_SIZE + 1];

    int32_t *address = get_address(vfs, inode);
    for (int i = 0; i < cluster_count; i++) {
        fseek(file, address[i], SEEK_SET);
        fread(buffer[i], ONE_CLUSTER_SIZE, 1, file);
        if (i == (cluster_count - 1)) buffer[i][strlen(buffer[i]) + 1] = '\0';
    }
    fclose(file);

    // Write
    FILE *file_dest = fopen(dest, "wb");
    if(file_dest == NULL)
    {
        printf("ERROR: Cannot open destination file %s\n", dest);
        return;
    }

    int actual_size = inode -> file_size;
    fseek(file_dest, 0, SEEK_SET);
    for (int i = 0; i < inode -> cluster_count; i++) {
        if (actual_size >= ONE_CLUSTER_SIZE) {
            fwrite(buffer[i], ONE_CLUSTER_SIZE, 1, file_dest);
            actual_size -= ONE_CLUSTER_SIZE;
        }
        else {
            fwrite(buffer[i], actual_size, 1, file_dest);
            fflush(file_dest);
            fputc(0x0a, file_dest);
            break;
        }
        fseek(file_dest, ONE_CLUSTER_SIZE, SEEK_SET);
        fflush(file_dest);
    }

    fflush(file_dest);
    fclose(file_dest);

    printf("INFO: OK\n");
}

void remove_file(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        printf("ERROR: Invalid directory name\n");
        return;
    }
    INODE *inode = find_inode_by_name(vfs, dir_name);
    if (inode == NULL) {
        printf("ERROR: %s", DIR_NOT_FOUND);
        return;
    }

    if (inode -> isDirectory) {
        printf("ERROR: cannot remove directory");
        return;
    }

    if (remove_file_from_fs(vfs, inode) == -1) {
        return;
    }
    printf("INFO: OK");
}

void copy_file(VFS **vfs, char *tok) {
    char *source = strtok(NULL, SPLITTER);
    if (source == NULL || strlen(source) <= 1) {
        printf("ERROR: Invalid source path\n");
        return;
    }

    char *dest = strtok(NULL, SPLITTER);
    if (dest == NULL || strlen(dest) <= 1) {
        printf("ERROR: Invalid destination path\n");
        return;
    }
    if (strlen(dest) > 0 && dest[strlen(dest) - 1] == '\n') dest[strlen(dest) - 1] = '\0';

    char *full_path = malloc(sizeof(char) * strlen(dest));
    strcpy(full_path, (*vfs) -> actual_path);
    strcat(full_path, "/");
    strcat(full_path, dest);

    char * token, * new_file;
    new_file = token = strtok(dest, "/");
    for (;(token = strtok(NULL, "/")) != NULL; new_file = token);

    // Cesta bez koncove slozky
    char *path = malloc(sizeof(char) * strlen(full_path));
    strcpy(path, full_path);
    path[strlen(full_path) - strlen(new_file)] = '\0';

    // Parent folder name
    char * t, * parent;
    parent = t = strtok(path, "/");
    for (;(t = strtok(NULL, "/")) != NULL; parent = t);


    INODE *source_inode = find_inode_by_name(vfs, source);
    if (source_inode == NULL) {
        printf("ERROR: cannot find inode with name '%s'", source);
        free(full_path);
        free(path);
        return;
    }

    INODE *dest_inode = find_inode_by_name(vfs, parent);
    if (dest_inode == NULL) {
        printf("ERROR: cannot find inode with name '%s'", parent);
        free(full_path);
        free(path);
        return;
    }


    INODE *test = find_inode_by_name(vfs, new_file);
    if (test != NULL) {
        printf("ERROR: file already exist in destination directory");
        free(full_path);
        free(path);
        return;
    }

    if (source_inode -> isDirectory) {
        printf("ERROR: cannot move directory");
        free(full_path);
        free(path);
        return;
    }

    if (!dest_inode -> isDirectory) {
        printf("ERROR: destination folder is file");
        free(full_path);
        free(path);
        return;
    }

    if (copy_file_in_directory(vfs, dest_inode, source_inode, new_file) == -1) {
        free(full_path);
        free(path);
        return;
    }
    free(full_path);
    free(path);
    printf("INFO: OK");
}

void move_file(VFS **vfs, char *tok) {
    char *source = strtok(NULL, SPLITTER);
    if (source == NULL || strlen(source) <= 1) {
        printf("ERROR: Invalid source path\n");
        return;
    }

    char *dest = strtok(NULL, SPLITTER);
    if (dest == NULL || strlen(dest) <= 1) {
        printf("ERROR: Invalid destination path\n");
        return;
    }
    if (strlen(dest) > 0 && dest[strlen(dest) - 1] == '\n') dest[strlen(dest) - 1] = '\0';

    INODE *source_inode = find_inode_by_name(vfs, source);
    if (source_inode == NULL) {
        printf("ERROR: cannot find inode with name '%s'", source);
        return;
    }

    INODE *dest_inode = find_inode_by_name(vfs, dest);
    if (dest_inode == NULL) {
        printf("ERROR: cannot find inode with name '%s'", dest);
        return;
    }

    if (source_inode -> isDirectory) {
        printf("ERROR: cannot move directory");
    }

    if (!dest_inode -> isDirectory) {
        printf("ERROR: destination folder is file");
    }

    // Check if destination folder contains source file
    if (is_folder_contains_item(vfs, dest_inode, source_inode) != NULL) {
        printf("ERROR: file already exist in destination directory");
        return;
    }

    if (move_file_into_folder(vfs, dest_inode, source_inode) == -1) {
        return;
    }

    printf("INFO: OK");
}

int run_commands_from_file(FILE **file, char *tok) {
    char *filename = strtok(NULL, " \n");
    (*file) = fopen(filename, "r");
    if ((*file) != NULL) {
        printf("OK\n");
        return 1;
    }
    else {
        printf("FILE NOT FOUND\n");
        return 0;
    }
}

void consistency_check(VFS **vfs, char *tok) {
    for (int i = 0; i < (*vfs) -> inode_table -> size; i++) {
        if ((*vfs) -> inode_table -> items[i] -> nodeid == -1) continue;
        char *source_file_name = get_inode_name(vfs, (*vfs) -> inode_table -> items[i] -> nodeid);
        int item_size = (*vfs) -> inode_table -> items[i] -> file_size;

        if ((*vfs) -> inode_table -> items[i] -> isDirectory) {
            printf("DIRECTORY: '%s' - ", source_file_name);
        } else {
            printf("FILE: '%s' - ", source_file_name);
        }

        int cluster_count = 1;
        if (item_size !=0) {
            cluster_count = item_size / ONE_CLUSTER_SIZE;
            if ((item_size % ONE_CLUSTER_SIZE) != 0) cluster_count++;
        }

        if ((*vfs) -> inode_table -> items[i] -> cluster_count != cluster_count) {
            printf("NOT OK\n");
        } else {
            printf("OK\n");
        }

        free(source_file_name);
    }
}

void test(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        printf("ERROR: Invalid directory name\n");
        return;
    }
    if (strlen(dir_name) > 0 && dir_name[strlen(dir_name) - 1] == '\n') dir_name[strlen(dir_name) - 1] = '\0';

    INODE *inode = find_inode_by_name(vfs, dir_name);
    if (inode == NULL) {
        printf("ERROR: cannot find inode with name '%s'", dir_name);
        return;
    }

    inode -> cluster_count--;
    fwrite_inode_block(vfs);
    fwrite_inode_item(vfs, inode -> nodeid);
}

int format(VFS **vfs, char *tok) {
    FILE *filesystem;
    char *size = strtok(NULL, SPLITTER);
    int32_t disk_size = get_size(size); // bytes
    if (disk_size == -1)
        return DISK_SIZE;

    // Check file existing
    if(!file_exists((*vfs) -> filename))
    {
        // Create file
        FILE *file;
        file = fopen((*vfs) -> filename, "wb");
        if(file == NULL)
        {
            printf("ERROR: Cannot create file %s\n", (*vfs) -> filename);
            return DISK_SIZE;
        }

        fclose(file);
    }

    // Open file
    filesystem = fopen((*vfs) -> filename, "r+b");
    if(filesystem == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return DISK_SIZE;
    }

    free_vfs(vfs);

    // Create VFS
    create_vfs_file(vfs, disk_size, filesystem);
    fclose(filesystem);
    hello((*vfs) -> superblock -> disk_size);

    // Create inode table
    if (inode_blocks_init(vfs) == -1) {
        return DISK_SIZE;
    }

    return disk_size;
}

void commands_help() {
    printf("+-----------------------------------------------------------------------------------------+\n");
    printf("\t%s - Copy file (%s s1 s2)\n", COPY_FILE, COPY_FILE);
    printf("\t%s - Move file (%s s1 s2)\n", MOVE_FILE, MOVE_FILE);
    printf("\t%s - Remove file (%s s1)\n", REMOVE_FILE, REMOVE_FILE);
    printf("\t%s - Make directory (%s s1)\n", MAKE_DIRECTORY, MAKE_DIRECTORY);
    printf("\t%s - Remove empty directory (%s s1)\n", REMOVE_EMPTY_DIRECTORY, REMOVE_EMPTY_DIRECTORY);
    printf("\t%s - List Files and Directories (%s s1)\n", PRINT_DIRECTORY, PRINT_DIRECTORY);
    printf("\t%s - Concatenate (%s s1)\n", PRINT_FILE, PRINT_FILE);
    printf("\t%s - Change directory (%s s1)\n", MOVE_TO_DIRECTORY, MOVE_TO_DIRECTORY);
    printf("\t%s - Present working directory (%s)\n", ACTUAL_DIRECTORY, ACTUAL_DIRECTORY);
    printf("\t%s - Inode info (%s s1)\n", MFT_ITEM_INFO, MFT_ITEM_INFO);
    printf("\t%s - Copy file from HD to my i-node system (%s s1 s2)\n", INCP, INCP);
    printf("\t%s - Copy file from my i-node system  to HD (%s s1 s2)\n", OUTCP, OUTCP);
    printf("\t%s - Load commands from file (%s s1)\n", LOAD_COMMANDS, LOAD_COMMANDS);
    printf("\t%s - File format (%s 10MB)\n", FILE_FORMATTING, FILE_FORMATTING);
    printf("\t%s - Exit my i-node system (%s)\n", EXIT, EXIT);
    printf("\t%s - Available commands (%s)\n", HELP, HELP);
    printf("+-----------------------------------------------------------------------------------------+\n");
}