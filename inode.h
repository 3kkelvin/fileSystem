#ifndef INODE_H
#define INODE_H
#include <stdbool.h>

#define MAX_FILES 100
#define MAX_FILENAME_LENGTH 60//檔名長度上限 包含副檔名 先設60確保block塞得下16個DirectoryEntry
#define BLOCK_NUMBER 64 
typedef struct FileSystem FileSystem;
//Inode 存Metadata
typedef struct Inode{
    int inode_index;                              // Inode編號
    bool isFile;                                  // 是檔案還是資料夾
    bool isUsed;                                  // 是否正在使用
    int size;                                     // 檔案大小
    int directBlocks[BLOCK_NUMBER];              // 直接指向的Block，最大資料量 ≈ 64KB
    int indirectBlock[BLOCK_NUMBER];             // 間接指向的Block，最大資料量 ≈ 16MB
    //Inode 總大小不能超過16.064MB 也就是 16064 Block（保守估計）
} Inode;

//只有資料夾有的字典 Key為文件名 value為Inode索引 用來維護資料夾上下級
typedef struct DirectoryEntry{
    char filename[MAX_FILENAME_LENGTH]; // 資料、資料夾名
    int inode_index;                    // Inode索引
} DirectoryEntry;

//檔案系統整體資料
typedef struct SuperBlock{
    int partition_size; // 分區大小
    int total_blocks;   // 總block數量
    int system_blocks;  // 系統區域占用的block數（固定，包含SuperBlock、bitmaps、inode table）
    int total_inodes;   // 總Inode數量
    int used_inodes;    // 已使用Inode數量
    int folder_inodes;  // 資料夾Inode數量
    int used_blocks;    // 已使用block數量
    int block_size;     // block大小
    char password[10];  // 密碼
} SuperBlock;
// 一些可以計算的值：

// 1. 可用於數據的 blocks：
//    total_blocks - used_blocks

// 2. 剩餘空間：
//    (total_blocks - used_blocks) * block_size

//初始化一個root
void init_root(FileSystem* fs);
int write_directory_entry(FileSystem* fs, Inode* inode, int current_block_index, DirectoryEntry* new_entry);
#endif