#ifndef ZOS_MAIN_H
#define ZOS_MAIN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPLITTER " "
#define COMMAND_LEN 100
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

#define SIGNATURE "JANELLE"
#define DESCRIPTOR "ZOS-2019"

// ================================== [COMMANDS] ==================================
#define COPY_FILE "cp"
#define MOVE_FILE "mv"
#define REMOVE_FILE "rm"
#define MAKE_DIRECTORY "mkdir"
#define REMOVE_EMPTY_DIRECTORY "rmdir"
#define PRINT_DIRECTORY "ls"
#define PRINT_FILE "cat"
#define MOVE_TO_DIRECTORY "cd"
#define ACTUAL_DIRECTORY "pwd"
#define MFT_ITEM_INFO "info"
#define INCP "incp"
#define OUTCP "outcp"
#define LOAD_COMMANDS "load"
#define FILE_FORMATTING "format"
#define CHECK "check"
#define EXIT "exit"
#define HELP "help"
#define SYSTEMINFO "sysinfo"
#define TEST "test"

#define ROOT_NAME "root"

// ================================== [CONSTANT] ==================================
#define DISK_SIZE 102400
#define ONE_CLUSTER_SIZE 4096
#define CLUSTER_COUNT 23
#define MAX_NAME_LEN 12
#define DIRECTORY 1
#define MY_FILE 0
#define MAX_DIR_COUNT 256
#define MAX_INODE_COUNT 100
#define MAX_DIRECT_LINKS 5

#define ID_ITEM_FREE -1


// ================================== [FILESYSTEM] ==================================

#define FILE_NF "FILE NOT FOUND"
#define PATH_NF "PATH NOT FOUND"
#define FILE_IS_DIR "IS NOT A DIRECTORY"
#define DIR_NOT_FOUND "FILE NOT FOUND"
#define EXIST "EXIST"
#define NE "NOT EMPTY"
#define CCFILE "CANNOT CREATE FILE"

typedef struct superblock SUPERBLOCK;
typedef struct pseudo_inode INODE;
typedef struct my_inode_array INODE_BLOCK;
typedef struct my_bitmap BITMAP;
typedef struct my_vfs VFS;
typedef struct directory_item DIR_ITEM;

struct my_vfs {
    SUPERBLOCK *superblock;
    INODE_BLOCK *inode_blocks;
    BITMAP *bitmap;

    char *actual_path;
    char *filename;
};

struct superblock {
    char signature[9];              //login autora FS
    char volume_descriptor[251];    //popis vygenerovaného FS
    int32_t disk_size;              //celkova velikost VFS
    int32_t cluster_size;           //velikost clusteru
    int32_t cluster_count;          //pocet clusteru
    int32_t inode_start_address;    //adresa pocatku  i-uzlů
    int32_t bitmap_start_address;   //adresa pocatku bitmapy datových bloků
    int32_t data_start_address;     //adresa pocatku datovych bloku
};

struct my_inode_array {
    int32_t size;
    INODE *items[MAX_INODE_COUNT];
};

struct pseudo_inode {
    int32_t nodeid;                 //ID i-uzlu, pokud ID = ID_ITEM_FREE, je polozka volna
    int isDirectory;                //soubor, nebo adresar
    int8_t references;              //počet odkazů na i-uzel, používá se pro hardlinky
    int32_t file_size;              //velikost souboru v bytech
    int32_t direct[MAX_DIRECT_LINKS]; // příme odkazy na datové bloky
    int32_t indirect1;              // 1. nepřímý odkaz (odkaz - datové bloky)
    int32_t indirect2;              // 2. nepřímý odkaz (odkaz - odkaz - datové bloky)
    int cluster_count;
};

struct my_bitmap {
    int32_t length;
    int data[CLUSTER_COUNT];
};

struct directory_item {
    int32_t inode;                   // inode odpovídající souboru
    char item_name[12];              //8+3 + /0 C/C++ ukoncovaci string znak
};


// MAIN
void hello(int disk_size);
int compare_two_string(char *string1, char *string2);
int getLine (char *prmpt, char *buff, size_t sz);
int get_folder_count(char *str);
int get_path_array (char *path, char *array[10]);
void removeChar(char *str, char garbage);
int file_exists(const char *fname);
int directory_exists(char *path);

// BITMAP
int32_t get_one_free_cluster(BITMAP **bitmap);
int check_free_clusters (BITMAP **bitmap, int count);
void bitmap_init(BITMAP **bitmap, int32_t cluster_count);
int bitmap_contains_free_cluster(BITMAP *bitmap);
int used_clusters(BITMAP *bitmap);
void bitmap_info(BITMAP *bitmap);
struct the_fragment_temp *find_free_cluster(BITMAP **bitmap, int needed_count);
int32_t set_one_bitmap_on_index (BITMAP **bitmap, int index);
int32_t set_zero_bitmap_on_index (BITMAP **bitmap, int index);
void fwrite_bitmap(VFS **vfs);
void fread_bitmap(VFS **vfs, FILE *file);

// VIRTUAL FILESYSTEM
void vfs_init(VFS **vfs, char *filename, size_t disk_size);
void create_vfs_file(VFS **vfs, size_t disk_size, FILE *file);
void go_to_parent_folder(VFS **vfs);
void systeminfo(VFS **vfs);

// SUPERBLOCK
void superblock_init(SUPERBLOCK **superblock, int32_t disk_size, int32_t cluster_size);
void superblock_info(SUPERBLOCK *superblock);
void fread_superblock(VFS **vfs, FILE *file);

// INODE BLOCKS
int inode_blocks_init(VFS **vfs);
int inode_init(VFS **vfs, int node_id, char *name, int isDirectory, int item_size, int32_t parent_node_id);
int fill_data_block_directory(VFS **vfs, int node_id, char *new_folder_name, int32_t parent_node_id);
int fill_data_block_file(VFS **vfs, char *name, int cluster_count, int node_id, int32_t parent_node_id);
int init_root_directory(VFS **vfs, int node_id);
int add_folder_to_structure(VFS **vfs, int node_id, char *dir_name, int32_t cluster_id,  int dir_item_id) ;
INODE *find_directory (VFS **vfs, char *dir_name);
INODE *get_inode_by_id (VFS **vfs, int32_t id);
int remove_directory (VFS **vfs, INODE *inode);
INODE *find_inode_by_name (VFS **vfs, char *name);
int get_free_dir_item_id (VFS **vfs, INODE *inode);
int remove_file_from_directory(VFS **vfs, int32_t parent_inode_id, int32_t inode_id);
INODE *directory_exist (VFS **vfs, char *actual_folder_name, char *new_folder_name);
void fwrite_inode_block(VFS **vfs);
void fwrite_inode_item(VFS **vfs, int node_id);
void fread_inode_block(VFS **vfs, FILE *file);
int make_file_in_inodes(VFS **vfs, char *source_name, char *dest_name, INODE *dest_inode);
int remove_file_from_fs (VFS **vfs, INODE *inode);
char *is_folder_contains_item (VFS **vfs, INODE *dest_inode, INODE *source_inode);
int copy_file_in_directory(VFS **vfs, INODE *dest_inode, INODE *source_inode, char *new_filename);
int move_file_into_folder (VFS **vfs, INODE *dest_inode, INODE *source_inode);
INODE *get_parent_inode (VFS **vfs, INODE *inode);
char *get_inode_name (VFS **vfs, int inode_id);

// COMMANDS
void actual_directory(VFS *vfs);
void make_directory(VFS **vfs, char *tok);

void copy_file(VFS **vfs, char *tok);
void move_file(VFS **vfs, char *tok);
void remove_file(VFS **vfs, char *tok);
void remove_empty_directory(VFS **vfs, char *tok);
void list_files_and_directories(VFS **vfs, char *tok);
void concatenate(VFS **vfs, char *tok);
void change_directory(VFS **vfs, char *tok);
void present_working_directory();
void inode_info(VFS **vfs, char *tok);
void hd_to_fs(VFS **vfs, char *tok);
void fs_to_hd(VFS **vfs, char *tok);
int run_commands_from_file(FILE **file, char *tok);
void consistency_check(VFS **vfs, char *tok);
void test(VFS **vfs, char *tok);
void commands_help();

void ls (VFS **vfs);
#endif //ZOS_MAIN_H
