#include <stdio.h>
#include <string.h>

#include "main.h"
#include "commands.h"

VFS *my_vfs;
char data_filename[] = "filesystem.dat";

int main() {
    char *command_part;
    int rc;
    char buff[COMMAND_LEN];

    hello();
    vfs_init(&my_vfs, data_filename, DISK_SIZE);

    while (1) {
        rc = getLine ("\n$ ", buff, sizeof(buff));
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
//            remove_file(&vfs, tok);
        }
        else if (compare_two_string(command_part, MAKE_DIRECTORY) == 0) {
            make_directory(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, REMOVE_EMPTY_DIRECTORY) == 0) {
//            remove_empty_directory(&vfs, tok);
        }
        else if (compare_two_string(command_part, PRINT_DIRECTORY) == 0) {
//            list_files_and_directories(vfs, tok);
        }
        else if (compare_two_string(command_part, PRINT_FILE) == 0) {
//            concatenate(vfs, tok);
        }
        else if (compare_two_string(command_part, MOVE_TO_DIRECTORY) == 0) {
//            change_directory(&vfs, tok);
        }
        else if (compare_two_string(command_part, ACTUAL_DIRECTORY) == 0) {
            actual_directory(my_vfs);
            //present_working_directory(vfs);
        }
        else if (compare_two_string(command_part, MFT_ITEM_INFO) == 0) {
//            inode_info(vfs, tok);
        }
        else if (compare_two_string(command_part, HD_TO_PSEUDO) == 0) {
//            hd_to_fs(&vfs, tok);
        }
        else if (compare_two_string(command_part, PSEUDO_TO_HD) == 0) {
//            fs_to_hd(&vfs, tok);
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
