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

void ls(FileSystem* fs, Inode* inode);
void read_directory_entries(FileSystem* fs, int block_index);
#endif