#include "space.h"
#include <string.h>

FileSystem* init_space(int partition_size) {
    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if (!fs) return NULL;

    // 分配原始空間
    fs->raw_space = (unsigned char*)malloc(partition_size);
    if (!fs->raw_space) {
        free(fs);
        return NULL;
    }

    // 計算各個部分的大小
    int total_blocks = partition_size / BLOCK_SIZE;
    int inode_blocks = total_blocks / 10;  // 分配10%的空間給inode表
    int remaining_blocks = total_blocks - 1 - inode_blocks;  // 減去超級塊

    // 分配整個檔案系統空間
    unsigned char* raw_space = (unsigned char*)malloc(partition_size);
    if (!raw_space) {
        free(fs);
        return NULL;
    }

    // 初始化超級塊
    fs->super_block = (SuperBlock*)raw_space;
    fs->super_block->partition_size = partition_size;
    fs->super_block->total_blocks = total_blocks;
    fs->super_block->used_blocks = 1 + inode_blocks;  // 超級塊 + inode表佔用的塊
    fs->super_block->block_size = BLOCK_SIZE;
    fs->super_block->total_inodes = (inode_blocks * BLOCK_SIZE) / sizeof(Inode);
    fs->super_block->used_inodes = 0;
    fs->super_block->files_blocks = 0;
    fs->super_block->free_space = remaining_blocks * BLOCK_SIZE;

    // 設置位圖
    int bitmap_size = (total_blocks + 7) / 8;  // 每個bit代表一個塊
    fs->block_bitmap = raw_space + BLOCK_SIZE;  // 位圖緊跟在超級塊後面
    memset(fs->block_bitmap, 0, bitmap_size);

    // 設置inode位圖和inode表
    int inode_bitmap_size = (fs->super_block->total_inodes + 7) / 8;
    fs->inode_bitmap = fs->block_bitmap + bitmap_size;
    fs->inode_table = (Inode*)(fs->inode_bitmap + inode_bitmap_size);
    memset(fs->inode_bitmap, 0, inode_bitmap_size);

    // 數據塊區域
    fs->data_blocks = raw_space + (1 + inode_blocks) * BLOCK_SIZE;

    // 標記已使用的塊
    for (int i = 0; i <= inode_blocks; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        fs->block_bitmap[byte_index] |= (1 << bit_index);
    }

    return fs;
}

int allocate_inode(FileSystem* fs) {
    for (int i = 0; i < fs->super_block->total_inodes; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        
        if (!(fs->inode_bitmap[byte_index] & (1 << bit_index))) {
            fs->inode_bitmap[byte_index] |= (1 << bit_index);
            fs->super_block->used_inodes++;
            return i;
        }
    }
    return -1;  // 沒有空閒inode
}

int allocate_block(FileSystem* fs) {
    int total_blocks = fs->super_block->total_blocks;
    
    // 查找第一個空閒塊
    for (int i = 0; i < total_blocks; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        
        if (!(fs->block_bitmap[byte_index] & (1 << bit_index))) {
            // 標記為已使用
            fs->block_bitmap[byte_index] |= (1 << bit_index);
            fs->super_block->used_blocks++;
            fs->super_block->free_space -= BLOCK_SIZE;
            return i;
        }
    }
    return -1;  // 沒有空閒塊
}

void free_inode(FileSystem* fs, int inode_number) {
    int byte_index = inode_number / 8;
    int bit_index = inode_number % 8;
    
    if (fs->inode_bitmap[byte_index] & (1 << bit_index)) {
        fs->inode_bitmap[byte_index] &= ~(1 << bit_index);
        fs->super_block->used_inodes--;
    }
}

void free_block(FileSystem* fs, int block_number) {
    int byte_index = block_number / 8;
    int bit_index = block_number % 8;
    
    // 只有當塊被標記為已使用時才釋放
    if (fs->block_bitmap[byte_index] & (1 << bit_index)) {
        fs->block_bitmap[byte_index] &= ~(1 << bit_index);
        fs->super_block->used_blocks--;
        fs->super_block->free_space += BLOCK_SIZE;
    }
}

void read_block(FileSystem* fs, int block_number, void* buffer) {
    memcpy(buffer, fs->data_blocks + block_number * BLOCK_SIZE, BLOCK_SIZE);
}

void write_block(FileSystem* fs, int block_number, const void* buffer) {
    memcpy(fs->data_blocks + block_number * BLOCK_SIZE, buffer, BLOCK_SIZE);
}

void destroy_space(FileSystem* fs) {
    if (fs) {
        free(fs->super_block);  // 這會釋放整個raw_space
        free(fs);
    }
}