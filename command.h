#ifndef COMMAND_H
#define COMMAND_H
#include <stdio.h>
typedef struct SuperBlock SuperBlock;
typedef struct FileSystem FileSystem;
typedef struct Inode Inode;

//列出可用指令
void print_command(void);
//返回指令對應enum數字
int get_command_code(const char *input);
//顯示目前fs資訊
void status(SuperBlock *status);
//列出當前路徑底下的路徑與檔案
void ls(FileSystem* fs, Inode* inode);
//跳至該路徑
Inode* cd(FileSystem* fs, Inode* inode, char *arg, char *text);
void mkdir(FileSystem* fs, Inode* inode, char *arg);
void read_directory_entries(FileSystem* fs, int block_index);
#endif