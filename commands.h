#ifndef ZOS_COMMANDS_H
#define ZOS_COMMANDS_H

#include "main.h"

void actual_directory(VFS *vfs);
void make_directory(VFS **vfs, char *tok);

void copy_file(char *tok);
void move_file(char *tok);
void remove_file(char *tok);
void remove_empty_directory(char *tok);
void list_files_and_directories(char *tok);
void concatenate(char *tok);
void change_directory(char *tok);
void present_working_directory();
void inode_info(char *tok);
void hd_to_fs(char *tok);
void fs_to_hd(char *tok);
int run_commands_from_file(char *tok);
void file_format(char *tok);
void commands_help();

#endif //ZOS_COMMANDS_H
