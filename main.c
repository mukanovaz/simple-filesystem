#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "main.h"

VFS *my_vfs;
char data_filename[] = "filesystem.dat";

int main() {
    char *command_part;
    int rc;
    char buff[COMMAND_LEN];

    hello();
    vfs_init(&my_vfs, data_filename, DISK_SIZE);

    while (1) {
        printf("\n%s ", my_vfs -> actual_path);
        rc = getLine ("$ ", buff, sizeof(buff));
        if (rc == NO_INPUT) {
            printf ("\nNo input\n");
            continue;
        }

        if (rc == TOO_LONG) {
            printf ("Input too long [%s]\n", buff);
            continue;
        }

        command_part = strtok(buff, SPLITTER);

        if (compare_two_string(command_part, COPY_FILE) == 0) {
//            copy_file(&vfs, tok);
        }
        else if (compare_two_string(command_part, MOVE_FILE) == 0) {
//            move_file(&vfs, tok);
        }
        else if (compare_two_string(command_part, REMOVE_FILE) == 0) {
            remove_file(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, MAKE_DIRECTORY) == 0) {
            make_directory(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, REMOVE_EMPTY_DIRECTORY) == 0) {
            remove_empty_directory(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, PRINT_DIRECTORY) == 0) {
            list_files_and_directories(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, PRINT_FILE) == 0) {
            concatenate(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, MOVE_TO_DIRECTORY) == 0) {
            change_directory(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, ACTUAL_DIRECTORY) == 0) {
            actual_directory(my_vfs);
        }
        else if (compare_two_string(command_part, MFT_ITEM_INFO) == 0) {
            inode_info(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, INCP) == 0) {
            hd_to_fs(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, OUTCP) == 0) {
            fs_to_hd(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, LOAD_COMMANDS) == 0) {
//            is_used_file = run_commands_from_file(&file_with_commands, tok);
        }
        else if (compare_two_string(command_part, FILE_FORMATTING) == 0) {
//            file_format(&vfs, tok);
        }
        else if (compare_two_string(command_part, HELP) == 0) {
//            commands_help();
        }
        else if (compare_two_string(command_part, EXIT) == 0) {
            break;
        }
        else
        {
            printf("Unknown command\n");
        }
    }
    return 0;
}

void hello() {
    printf("+-----------------------------------+\n");
    printf("|   [I-NODE FILESYSTEM SIMULATION]  |\n");
    printf("|          [MUKANOVA ZHANEL]        |\n");
    printf("+-----------------------------------+\n");
}

int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return OK;
}

int compare_two_string(char *string1, char *string2) {
    char s1[strlen(string1)];
    char s2[strlen(string2)];

    strcpy(s1, string1);
    strcpy(s2, string2);

    if (strlen(s1) > 0 && s1[strlen(s1) - 1] == '\n') s1[strlen(s1) - 1] = '\0';
    if (strlen(s2) > 0 && s2[strlen(s2) - 1] == '\n') s2[strlen(s2) - 1] = '\0';

    if (strlen(s1) != strlen(s2)) return 1;
    else {
        int i;
        for (i = 0; i < strlen(s1); i++) {
            if (s1[i] != s2[i]) return 1;
        }
        return 0;
    }
}

int get_folder_count(char *str) {
    char path[strlen(str)];
    strcpy(path, str);
    char *ttok = strtok(path, "/");
    int folders_count = 0;
    while(ttok != NULL) {
        ttok = strtok(NULL, "/");
        folders_count++;
    }
    return folders_count;
}

int get_path_array (char *path, char *array[10]) {
    int i=0, array_size = 0;

    char string[strlen(path)];
    strcpy(string, path);
    array[i] = strtok(string,"/");

    while(array[i]!=NULL)
    {
        array[++i] = strtok(NULL,"/");
        array_size++;
    }
    return array_size;
}

void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

int file_exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}


int directory_exists(char *path) {
    struct stat s;
    int err = stat(path, &s);
    if(-1 == err) {
        if(ENOENT == errno) {
            return 0;
        }
        else return 0;
    }
    else {
        if(S_ISDIR(s.st_mode)) {
            return 1;
        }
        else {
            return 0;
        }
    }
    return 0;
}