// space.h
#ifndef SPACE_H
#define SPACE_H

#include "inode.h"
#include <stdlib.h>

#define BLOCK_SIZE 1024  // 1KB per block

// 分配的記憶體空間結構
typedef struct {
    SuperBlock* super_block;     // 指向超級塊
    Inode* inode_table;          // 指向inode表
    unsigned char* data_blocks;   // 指向數據塊區域
    unsigned char* data_bitmap;  // 塊位圖，用於追踪數據塊的使用情況
    unsigned char* inode_bitmap;  // inode位圖，用於追踪inode的使用情況
    unsigned char* raw_space;     // 指向整個原始空間的起始位置
} FileSystem;

// 初始化檔案系統
FileSystem* init_space(int partition_size);

// 分配一個新的數據塊，返回塊號，失敗返回-1
int allocate_block(FileSystem* fs);

// 釋放一個數據塊
void free_block(FileSystem* fs, int block_number);

// 分配一個新的inode，返回inode號，失敗返回-1
int allocate_inode(FileSystem* fs);

// 釋放一個inode
void free_inode(FileSystem* fs, int inode_number);

// 讀取指定塊的數據
void read_block(FileSystem* fs, int block_number, void* buffer);

// 寫入數據到指定塊
void write_block(FileSystem* fs, int block_number, const void* buffer);

// 銷毀檔案系統，釋放所有記憶體
void destroy_space(FileSystem* fs);

#endif