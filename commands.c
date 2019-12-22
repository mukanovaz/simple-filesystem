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

    char *full_path = malloc(sizeof(char) * strlen(ptr));
    strcpy(full_path, (*vfs) -> actual_path);
    strcat(full_path, "/");
    strcat(full_path, ptr);

    // Posledni token
    char * token, * last;
    last = token = strtok(ptr, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);

    // Cesta bez koncove slozky
    char *path = malloc(sizeof(char) * strlen(full_path));
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
        if(inode_init(vfs, (*vfs) -> inode_blocks -> size, last, DIRECTORY, DIRECTORY, parent_inode -> nodeid) == -1) {
            printf("ERROR: cannot create directory '%s'", last);
            return;
        }
        printf("OK");
    } else {
        printf("ERROR: A folder with name '%s' already exists\n", last);
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

        INODE *tmp = get_inode_by_id(vfs, item -> inode);
        if (tmp == NULL) break;
        if (tmp -> isDirectory == 1) {
            printf("\t+ ");
        } else {
            printf("\t- ");
        }
        printf("%s\n", item -> item_name);
        free(item);
    }
    fclose(file);
}

void change_directory(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        return;
    }

    // Back
    if (strncmp(dir_name, "..", 2) == 0) {
        // Back
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
        FILE *file = fopen((*vfs)->filename, "r+b");
        int32_t max_dir_count = ONE_CLUSTER_SIZE / sizeof(DIR_ITEM);
        int32_t block_start_addr = inode->direct[0];

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

    for (int i = 0; i < inode->cluster_count; i++) {
        printf("| INODE direct[%d]: %d\n", i, inode -> direct[i]);
    }

    if (inode -> cluster_count <= 5) {
      // print indirect
    }
    printf("+-----------------------------------------------------------------------------------------+\n");
}

void hd_to_fs(VFS **vfs, char *tok) {
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

    // Check if source exist
    FILE *file_src = fopen(dir_name, "rb");
    if (file_src == NULL) {
        printf("ERROR: cannot open a file %s\n", dir_name);
        return;
    }
    fclose(file_src);

    char *full_path = malloc(sizeof(char) * strlen(dest));
    strcpy(full_path, (*vfs) -> actual_path);
    strcat(full_path, "/");
    strcat(full_path, dest);

    // Posledni token
    char * token, * last;
    last = token = strtok(dest, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);

    // Cesta bez koncove slozky
    char *path = malloc(sizeof(char) * strlen(full_path));
    strcpy(path, full_path);
    path[strlen(full_path) - strlen(last)] = '\0';

    INODE *destination = find_directory(vfs, path);
    if (destination == NULL) {
        return;
    }

    // Create file in pseudo FS
    make_file_in_inodes(vfs, dir_name, last, destination);
}


void concatenate(VFS **vfs, char *tok) {
    char buffer[ONE_CLUSTER_SIZE];
    int i, j;
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
    file = fopen((*vfs) -> filename, "r+b");
    if(file == NULL)
    {
        printf("ERROR: Cannot open file %s\n", (*vfs) -> filename);
        return;
    }

    printf("\n");
    for (i = 0; i < inode -> cluster_count; i++) {
        // TODO: add indirect
        int cluster_id = (inode -> direct[i] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
        int32_t data_block_addr = (*vfs) -> superblock -> data_start_address + (ONE_CLUSTER_SIZE * cluster_id);

        fseek(file, data_block_addr, SEEK_SET);
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
    printf("\n");
    fclose(file);
}

void fs_to_hd(VFS **vfs, char *tok) {
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

    FILE *file_dest = fopen(dest, "wb");
    if(file_dest == NULL)
    {
        printf("ERROR: Cannot open destination file %s\n", (*vfs) -> filename);
        return;
    }

    if (inode -> file_size > 0) {
        int i, j;
        int position = 0;
        char buffer[ONE_CLUSTER_SIZE];
        int32_t actual_size = inode -> file_size;

        for (i = 0; i < inode -> cluster_count; i++) {
            // TODO: add indirect
            int cluster_id = (inode -> direct[i] - (*vfs) -> superblock -> data_start_address) / ONE_CLUSTER_SIZE;
            int32_t data_block_addr = (*vfs) -> superblock -> data_start_address + (ONE_CLUSTER_SIZE * cluster_id);

            fseek(file, data_block_addr, SEEK_SET);
            fread(buffer, ONE_CLUSTER_SIZE, 1, file);
            fseek(file_dest, position * ONE_CLUSTER_SIZE, SEEK_SET);
            position++;
            if (actual_size >= ONE_CLUSTER_SIZE) {
                actual_size -= ONE_CLUSTER_SIZE;
                fwrite(buffer, ONE_CLUSTER_SIZE, 1, file_dest);
            }
            else {
                fwrite(buffer, actual_size - 1, 1, file_dest);
                fflush(file_dest);
                fputc(0x0a, file_dest); //LF
                break;
            }
            fflush(file_dest);

        }
    }
    fflush(file_dest);
    fclose(file_dest);
    fclose(file);
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