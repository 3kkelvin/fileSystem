#include "space.h"
#include <string.h>

FileSystem* init_space(int partition_size) {
    // 1. 基本檢查和分配
    if (partition_size <= 0 || partition_size % BLOCK_SIZE != 0) {
        return NULL;
    }

    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if (!fs) return NULL;

    fs->raw_space = (unsigned char*)malloc(partition_size);
    if (!fs->raw_space) {
        free(fs);
        return NULL;
    }

    // 2. 空間規劃
    int total_blocks = partition_size / BLOCK_SIZE;
    int inode_blocks = total_blocks / 10;  // 10% 給 inode 表
    int total_inodes = (inode_blocks * BLOCK_SIZE) / sizeof(Inode);
    
    // 決定各個bitmap的大小
    int inode_bitmap_size = (total_inodes + 7) / 8;
    
    // 3. 預先計算剩餘可用的data blocks（先扣掉inode table佔用的空間）
    int available_blocks = total_blocks - inode_blocks;
    int data_bitmap_size = (available_blocks + 7) / 8;

    // 4. 計算系統區域（SuperBlock + bitmaps）需要多少blocks
    size_t system_metadata_size = sizeof(SuperBlock) + inode_bitmap_size + data_bitmap_size;
    int metadata_blocks = (system_metadata_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // 5. 最終得到真正可用於數據存儲的blocks數量
    int data_blocks = total_blocks - inode_blocks - metadata_blocks;

    // 6. 設置指針位置
    fs->super_block = (SuperBlock*)fs->raw_space;
    fs->inode_bitmap = fs->raw_space + sizeof(SuperBlock);
    fs->data_bitmap = fs->inode_bitmap + inode_bitmap_size;
    fs->inode_table = (Inode*)(fs->raw_space + (metadata_blocks * BLOCK_SIZE));
    fs->data_blocks = fs->raw_space + ((metadata_blocks + inode_blocks) * BLOCK_SIZE);

    // 7. 初始化 SuperBlock
    fs->super_block->partition_size = partition_size;
    fs->super_block->total_blocks = total_blocks;
    fs->super_block->system_blocks = metadata_blocks + inode_blocks;    // 系統區域占用的blocks
    fs->super_block->total_inodes = total_inodes;
    fs->super_block->used_inodes = 0;
    fs->super_block->block_size = BLOCK_SIZE;
    fs->super_block->used_blocks = metadata_blocks + inode_blocks;  // 系統佔用的區塊
    // 設定密碼
    strncpy(fs->super_block->password, "mmslab406", sizeof(fs->super_block->password) - 1);
    fs->super_block->password[sizeof(fs->super_block->password) - 1] = '\0';

    // 8. 初始化 bitmaps
    memset(fs->inode_bitmap, 0, inode_bitmap_size);
    memset(fs->data_bitmap, 0, data_bitmap_size);

    return fs;
}

Inode* allocate_inode(FileSystem* fs, bool isFile) {
    for (int i = 0; i < fs->super_block->total_inodes; i++) {
        int byte = i / 8;
        int bit = i % 8;

        if (!(fs->inode_bitmap[byte] & (1 << bit))) {
            // 2. 標記該 inode 為已使用
            fs->inode_bitmap[byte] |= (1 << bit);
            fs->super_block->used_inodes++;

            // 3. 初始化該 inode
            Inode* inode = &fs->inode_table[i];
            inode->inode_index = i;
            inode->isFile = isFile;
            inode->isUsed = true;
            inode->size = 0;

            // 初始化所有 block 指針為 -1（表示未使用）
            for (int i = 0; i < BLOCK_NUMBER; i++) {
                inode->directBlocks[i] = -1;
                inode->indirectBlock[i] = -1;
                inode->doubleIndirectBlock[i] = -1;
            }

            return inode;
        }
    }

    return NULL;  // 沒有空閒inode
}

int allocate_data_block(FileSystem* fs) {
    // 計算可用於數據的總blocks
    int available_blocks = fs->super_block->total_blocks - fs->super_block->used_blocks;
    
    // 檢查是否還有可用空間
    if (available_blocks <= 0) {
        return -1;
    }

    int data_blocks = fs->super_block->total_blocks - fs->super_block->system_blocks;

    // 遍歷 data bitmap 找尋空閒的 block
    for (int i = 0; i < data_blocks; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        
        if (!(fs->data_bitmap[byte_index] & (1 << bit_index))) {
            // 標記為已使用
            fs->data_bitmap[byte_index] |= (1 << bit_index);
            // 更新已使用block計數
            fs->super_block->used_blocks++;
            return i;
        }
    }
    
    return -1;  // 沒有空閒的 data block
}

// int allocate_block(FileSystem* fs) {
//     int total_blocks = fs->super_block->total_blocks;
    
//     // 查找第一個空閒塊
//     for (int i = 0; i < total_blocks; i++) {
//         int byte_index = i / 8;
//         int bit_index = i % 8;
        
//         if (!(fs->block_bitmap[byte_index] & (1 << bit_index))) {
//             // 標記為已使用
//             fs->block_bitmap[byte_index] |= (1 << bit_index);
//             fs->super_block->used_blocks++;
//             fs->super_block->free_space -= BLOCK_SIZE;
//             return i;
//         }
//     }
//     return -1;  // 沒有空閒塊
// }

// void free_inode(FileSystem* fs, int inode_number) {
//     int byte_index = inode_number / 8;
//     int bit_index = inode_number % 8;
    
//     if (fs->inode_bitmap[byte_index] & (1 << bit_index)) {
//         fs->inode_bitmap[byte_index] &= ~(1 << bit_index);
//         fs->super_block->used_inodes--;
//     }
// }

// void free_block(FileSystem* fs, int block_number) {
//     int byte_index = block_number / 8;
//     int bit_index = block_number % 8;
    
//     // 只有當塊被標記為已使用時才釋放
//     if (fs->block_bitmap[byte_index] & (1 << bit_index)) {
//         fs->block_bitmap[byte_index] &= ~(1 << bit_index);
//         fs->super_block->used_blocks--;
//         fs->super_block->free_space += BLOCK_SIZE;
//     }
// }

// void read_block(FileSystem* fs, int block_number, void* buffer) {
//     memcpy(buffer, fs->data_blocks + block_number * BLOCK_SIZE, BLOCK_SIZE);
// }

// void write_block(FileSystem* fs, int block_number, const void* buffer) {
//     memcpy(fs->data_blocks + block_number * BLOCK_SIZE, buffer, BLOCK_SIZE);
// }

// void destroy_space(FileSystem* fs) {
//     if (fs) {
//         free(fs->super_block);  // 這會釋放整個raw_space
//         free(fs);
//     }
// }