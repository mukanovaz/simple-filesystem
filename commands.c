#include <values.h>
#include "commands.h"
#include "main.h"

void actual_directory(VFS *vfs) {

    char path[strlen(vfs -> actual_path)];
    if (strlen(vfs -> actual_path) > 0) strcpy(path, vfs -> actual_path);
    else strcpy(path, "/");
    printf("%s", path);
}

void make_directory(VFS **vfs, char *tok) {
    char *dir_name = strtok(NULL, SPLITTER);
    if (dir_name == NULL || strlen(dir_name) <= 1) {
        printf("ERROR: Invalid directory name\n");
        return;
    }

    INODE *inode = find_directory(vfs, dir_name);
    if (inode == NULL) {
        inode_init(vfs, (*vfs) -> inode_blocks -> size, dir_name, DIRECTORY, DIRECTORY);
        ls(vfs);
        printf("OK");
    } else {
        printf("ERROR: A folder with name '%s' already exists\n", dir_name);
        return;
    }
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
    printf("\t%s - Copy file from HD to my i-node system (%s s1 s2)\n", HD_TO_PSEUDO, HD_TO_PSEUDO);
    printf("\t%s - Copy file from my i-node system  to HD (%s s1 s2)\n", PSEUDO_TO_HD, PSEUDO_TO_HD);
    printf("\t%s - Load commands from file (%s s1)\n", LOAD_COMMANDS, LOAD_COMMANDS);
    printf("\t%s - File format (%s 10MB)\n", FILE_FORMATTING, FILE_FORMATTING);
    printf("\t%s - Exit my i-node system (%s)\n", EXIT, EXIT);
    printf("\t%s - Available commands (%s)\n", HELP, HELP);
    printf("+-----------------------------------------------------------------------------------------+\n");
}