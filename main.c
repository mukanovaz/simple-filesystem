#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <values.h>

#include "main.h"

VFS *my_vfs;
char data_filename[] = "filesystem.dat";
int commands = 0;
FILE *commands_file;

int main(int argc, char *argv[]) {
    char *command_part;
    int rc;
    char buff[COMMAND_LEN];

    if (argc == 1)
    {
        vfs_init(&my_vfs, data_filename, DISK_SIZE);
    } else {
        vfs_init(&my_vfs, data_filename, atoi(argv[1]));
    }

    while (1) {
        printf("\n%s $ ", my_vfs -> actual_path);
        if (commands) {
            fgets(buff, COMMAND_LEN, commands_file);

            // Delete \n
            if (strlen(buff) > 0 && buff[strlen(buff) - 1] == '\n') buff[strlen(buff) - 1] = '\0';
            if (feof(commands_file) == 1) { //check end of file
                fclose(commands_file);
                commands = 0;
            }

            printf("%s\n", buff);
        } else {
            rc = getLine ("", buff, sizeof(buff));
            if (rc == NO_INPUT) {
                printf ("\nNo input\n");
                continue;
            }

            if (rc == TOO_LONG) {
                printf ("Input too long [%s]\n", buff);
                continue;
            }
        }

        command_part = strtok(buff, SPLITTER);
        if (command_part == NULL) {
            continue;
        }

        if (compare_two_string(command_part, COPY_FILE) == 0) {
            copy_file(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, MOVE_FILE) == 0) {
            move_file(&my_vfs, command_part);
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
            incp(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, OUTCP) == 0) {
            outcp(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, TEST) == 0) {
            test(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, LOAD_COMMANDS) == 0) {
            commands = run_commands_from_file(&commands_file, command_part);
        }
        else if (compare_two_string(command_part, CHECK) == 0) {
            consistency_check(&my_vfs, command_part);
        }
        else if (compare_two_string(command_part, HELP) == 0) {
            commands_help();
        }
        else if (compare_two_string(command_part, SYSTEMINFO) == 0) {
            systeminfo(&my_vfs);
        } else if (compare_two_string(command_part, FILE_FORMATTING) == 0) {
            format(&my_vfs, command_part);
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

void hello(int disk_size) {
    printf("+-----------------------------------+\n");
    printf("    [I-NODE FILESYSTEM SIMULATION]  \n");
    printf("           [MUKANOVA ZHANEL]        \n");
    printf("         [DISK SIZE: %d]          \n", disk_size);
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

int32_t get_size(char *size) {
    char *units = NULL;
    long number;

    if (!size ||  strcmp(size , "") == 0 ) {
        printf("ERROR: cannot create file");
        return -1;
    }

    number = strtol(size, &units, 0);	// Convert to number

    if (number == 0) {
        printf("ERROR: cannot create file");
        return -1;
    }

    if (strncmp("KB", units, 2) == 0) {			// Kilobytes
        number *= 1000;
    }
    else if (strncmp("MB", units, 2) == 0) {	// Megabytes
        number *= 1000000;
    }
    else if (strncmp("GB", units, 2) == 0) {	// Gigabytes
        number *= 1000000000;
    }

    if (number < MIN_FS_SIZE) {			// If the size is not enough large
        printf("ERROR: cannot create file");
        return -1;
    }
    else if (number > INT_MAX) {	// If the size is too large
        printf("ERROR: cannot create file");
        return -1;
    }

    return (int32_t)number;
}