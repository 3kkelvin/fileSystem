#ifndef COMMAND_H
#define COMMAND_H
#include <stdio.h>
#include <sys/stat.h>
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
void read_directory_entries(FileSystem* fs, int block_index);
//跳至該路徑
Inode* cd(FileSystem* fs, Inode* inode, char *arg, char *text);
//刪除檔案
void rm(FileSystem* fs, Inode *inode, char *arg);
//創路徑
void my_mkdir(FileSystem* fs, Inode* inode, char *arg);
//刪路徑
void my_rmdir(FileSystem* fs, Inode* inode, char *arg);
//從真實路徑讀檔案
void put(FileSystem* fs, Inode *inode, char *arg);
//寫檔案到真實路徑
void get(FileSystem* fs, Inode *inode, char *arg);
//print出檔案內容
void cat(FileSystem* fs, Inode *inode, char *arg);
#endif