cmake_minimum_required(VERSION 3.15)
project(ZOS C)

set(CMAKE_C_STANDARD 99)

add_executable(ZOS main.c main.h commands.c commands.h bitmap.c vfs.c superblock.c inode_blocks.c data_blocks.c)