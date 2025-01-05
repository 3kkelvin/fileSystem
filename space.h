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
    unsigned char* data_blocks;   // 指向資料塊區域
    unsigned char* data_bitmap;  // 塊位圖，用於追踪資料塊的使用情況
    unsigned char* inode_bitmap;  // inode位圖，用於追踪inode的使用情況
    unsigned char* raw_space;     // 指向整個原始空間的起始位置
} FileSystem;

// Note: 注明private的函數是不對外暴露的，僅供內部調用

// 初始化檔案系統, 返回檔案系統指針，失敗返回NULL
FileSystem* init_space(int partition_size);

// 分配一個新的inode，返回塊號，失敗返回NULL
Inode* allocate_inode(FileSystem* fs, bool isFile);

// private
// 分配一個新的data block，返回塊號，失敗返回-1
int allocate_data_block(FileSystem* fs); 

// 取得一個使用中inode的位置，如果不存在或是未使用返回NULL
Inode* get_inode(FileSystem* fs, int inode_index);

// 取得一個data block的位置, 如果不存在回NULL
unsigned char* get_block_position(FileSystem* fs, int block_index);

// 為 inode 分配一個新的 data block，返回block index，如果空間不足或是inode data block已滿，返回-1
int allocate_single_block_for_inode(FileSystem* fs, Inode* inode);

// private
// 分配一個新的int array block，並初始化為-1
int allocate_empty_int_array_block(FileSystem* fs);

// private
// 分配一個新的data block並設置到direct block中，返回block index，如果空間不足或是direct block已滿，返回-1
int allocate_data_block_for_direct_block(FileSystem* fs, int* directBlocks, int size);

// private
// 分配一個新的data block並設置到indirect block中，返回block index，如果空間不足或是indirect block已滿，返回-1
int allocate_data_block_for_indirect_block(FileSystem* fs, int* indirectBlock, int size);

// 釋放一個inode
bool free_inode(FileSystem* fs, int inode_index);

// 釋放一個data block
bool free_data_block(FileSystem* fs, int block_index);

// private
// 根據size為inode分配data block，如果空間不足或是inode data block已滿，返回false
bool allocate_block_by_size_for_inode(FileSystem* fs, Inode* inode, size_t size);

// private
// 寫入資料到指定data block
void write_block(FileSystem* fs, int block_index, const void* data, size_t size);

// 寫入資料到指定inode，如果空間不足或是inode data block已滿，返回false
bool write_file_data(FileSystem* fs, Inode* inode, const void* data, size_t size);

// private
// 寫入direct block
void write_direct_block(FileSystem* fs, int* directBlocks, const char** data_ptr, size_t* remaining_size);

// private
// 寫入indirect block
void write_indirect_block(FileSystem* fs, int* indirectBlock, const char** data_ptr, size_t* remaining_size);

// private
// 讀取指定data block的資料
void read_block(FileSystem* fs, int block_index, void* buffer, size_t size);

// 讀取指定inode的資料，輸入buffer指標儲存資料，返回讀取的資料大小
size_t read_file_data(FileSystem* fs, Inode* inode, void* buffer, size_t size);

// private
// 讀取direct block
void read_direct_block(FileSystem* fs, int* directBlocks, char** buf_ptr, size_t* remaining_size, size_t* total_read);

// private
// 讀取indirect block
void read_indirect_block(FileSystem* fs, int* indirectBlock, char** buf_ptr, size_t* remaining_size, size_t* total_read);

#endif