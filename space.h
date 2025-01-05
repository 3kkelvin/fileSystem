// space.h
#ifndef SPACE_H
#define SPACE_H
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define BLOCK_SIZE 1024  // 1KB per block
typedef struct SuperBlock SuperBlock;
typedef struct Inode Inode;
// 分配的記憶體空間結構
typedef struct FileSystem{
    SuperBlock* super_block;     // 指向超級塊
    Inode* inode_table;          // 指向inode表
    unsigned char* data_blocks;   // 指向數據塊區域
    unsigned char* data_bitmap;  // 塊位圖，用於追踪數據塊的使用情況
    unsigned char* inode_bitmap;  // inode位圖，用於追踪inode的使用情況
    unsigned char* raw_space;     // 指向整個原始空間的起始位置
} FileSystem;

// 初始化檔案系統
FileSystem* init_space(int partition_size);

// 分配一個新的inode，返回塊號，失敗返回NULL
Inode* allocate_inode(FileSystem* fs, bool isFile);

// 分配一個新的data block，返回塊號，失敗返回-1
int allocate_data_block(FileSystem* fs); 

// 取得一個使用中inode的位置，如果不存在或是未使用返回NULL
Inode* get_inode(FileSystem* fs, int inode_index);

// 取得一個data block的位置
unsigned char* get_block_position(FileSystem* fs, int block_index);

// 為 inode 分配一個新的 data block
int allocate_single_block_for_inode(FileSystem* fs, Inode* inode);

// 分配一個新的int array block，並初始化為-1
int allocate_empty_int_array_block(FileSystem* fs);

// 分配一個新的data block，並設置到direct block中
int allocate_data_block_for_direct_block(FileSystem* fs, int* directBlocks, int size);

// 分配一個新的data block，並設置到indirect block中
int allocate_data_block_for_indirect_block(FileSystem* fs, int* indirectBlock, int size);

// 釋放一個inode
bool free_inode(FileSystem* fs, int inode_index);

// 釋放一個data block
bool free_data_block(FileSystem* fs, int block_index);

// 根據size為inode分配data block
bool allocate_block_by_size_for_inode(FileSystem* fs, Inode* inode, size_t size)

// // 讀取指定塊的數據
// void read_block(FileSystem* fs, int block_number, void* buffer);

// // 寫入數據到指定塊
// void write_block(FileSystem* fs, int block_number, const void* buffer);

// // 銷毀檔案系統，釋放所有記憶體
// void destroy_space(FileSystem* fs);

#endif