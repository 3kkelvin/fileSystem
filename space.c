#include "space.h"
#include "inode.h"

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
    // 設定解密識別字元
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
            // 清空這個block（全部設為0）
            unsigned char* block_ptr = get_block_position(fs, i);
            memset(block_ptr, 0, BLOCK_SIZE);
            return i;
        }
    }
    
    return -1;  // 沒有空閒的 data block
}

Inode* get_inode(FileSystem* fs, int inode_index) {
    // 檢查參數有效性
    if (inode_index < 0 || inode_index >= fs->super_block->total_inodes) {
        return NULL;
    }
    
    Inode* inode = &fs->inode_table[inode_index];
    if (!inode->isUsed) {
        return NULL;
    }
    
    return inode;
}

unsigned char* get_block_position(FileSystem* fs, int block_index) {
    // 檢查參數有效性
    if (block_index < 0 || block_index >= (fs->super_block->total_blocks - fs->super_block->system_blocks)) {
        return NULL;
    }
    
    return fs->data_blocks + (block_index * BLOCK_SIZE);
}

int allocate_single_block_for_inode(FileSystem* fs, Inode* inode) {
    // 1. 先檢查 direct blocks
    int result = allocate_data_block_for_direct_block(fs, inode->directBlocks, BLOCK_NUMBER);
    if (result != -1) {
        return result;
    }
    
    // 2. direct blocks 都滿了，先暫時返回失敗
    result = allocate_data_block_for_indirect_block(fs, inode->indirectBlock, BLOCK_NUMBER);
    if (result != -1) {
        return result;
    }

    return -1;
}

int allocate_empty_int_array_block(FileSystem* fs) {
    int new_block = allocate_data_block(fs);
    if (new_block != -1) {
        // 清空這個block（全部設為-1）
        int* block_ptr = (int*)get_block_position(fs, new_block);
        for (int j = 0; j < BLOCK_SIZE/sizeof(int); j++) {
            block_ptr[j] = -1;
        }
    }
    return new_block;
}

int allocate_data_block_for_direct_block(FileSystem* fs, int* directBlocks, int size) {
    for (int i = 0; i < size; i++) {
        if (directBlocks[i] == -1) {
            // 找到空的 direct block slot，分配新的 block
            int new_block = allocate_data_block(fs);
            if (new_block != -1) {
                directBlocks[i] = new_block;
                return new_block;
            }
            return -1;  // 沒有可用的 data block
        }
    }
    return -1;  // 沒有可用的 direct block slot
}

int allocate_data_block_for_indirect_block(FileSystem* fs, int* indirectBlock, int size) {
    for (int i = 0; i < size; i++) {
        if (indirectBlock[i] == -1) {
            // 需要先分配一個block來存放指針
            int indirect_block = allocate_empty_int_array_block(fs);
            if (indirect_block == -1) return -1;
            
            // 分配一個新的data block
            int new_block = allocate_data_block(fs);
            if (new_block == -1) {
                free_data_block(fs, indirect_block);
                return -1;
            }
            
            int* indirect_block_ptr = (int*)get_block_position(fs, indirect_block);

            // 設置指針
            indirectBlock[i] = indirect_block;
            indirect_block_ptr[0] = new_block;
            return new_block;
        } else {
            // 已有indirect block，檢查是否有空間
            int* indirect_block_ptr = (int*)get_block_position(fs, indirectBlock[i]);
            int result = allocate_data_block_for_direct_block(fs, indirect_block_ptr, BLOCK_SIZE/sizeof(int));
            if (result != -1) {
                return result;
            }
        }
    }
    return -1;  // 所有indirect blocks都滿
}

bool free_inode(FileSystem* fs, int inode_index) {
    // 1. 檢查參數有效性
    if (inode_index < 0 || inode_index >= fs->super_block->total_inodes) {
        return false;
    }

    // 2. 獲取inode
    Inode* inode = &fs->inode_table[inode_index];
    if (!inode->isUsed) return false;  // 已經是空閒的

    // 3. 釋放所有direct blocks
    for (int i = 0; i < BLOCK_NUMBER; i++) {
        if (inode->directBlocks[i] != -1) {
            free_data_block(fs, inode->directBlocks[i]);
        }
    }

    // 4. 釋放所有indirect blocks
    for (int i = 0; i < BLOCK_NUMBER; i++) {
        if (inode->indirectBlock[i] != -1) {
            // 先釋放這個indirect block指向的所有blocks
            int* indirect_block_ptr = (int*)get_block_position(fs, inode->indirectBlock[i]);
            for (int j = 0; j < BLOCK_SIZE/sizeof(int); j++) {
                if (indirect_block_ptr[j] != -1) {
                    free_data_block(fs, indirect_block_ptr[j]);
                }
            }
            // 再釋放indirect block本身
            free_data_block(fs, inode->indirectBlock[i]);
        }
    }

    // 5. 在bitmap中標記為未使用
    int byte_index = inode_index / 8;
    int bit_index = inode_index % 8;
    fs->inode_bitmap[byte_index] &= ~(1 << bit_index);
    fs->super_block->used_inodes--;

    return true;
}

bool free_data_block(FileSystem* fs, int block_index) {
    // 1. 檢查參數有效性
    if (block_index < 0 || block_index >= (fs->super_block->total_blocks - fs->super_block->system_blocks)) {
        return false;
    }

    // 2. 計算在 bitmap 中的位置
    int byte_index = block_index / 8;
    int bit_index = block_index % 8;

    // 3. 檢查這個 block 是否已經是空閒的
    if (!(fs->data_bitmap[byte_index] & (1 << bit_index))) {
        return false;  // 已經是空閒的，不需要釋放
    }
    
    // 4. 在 bitmap 中標記為未使用
    fs->data_bitmap[byte_index] &= ~(1 << bit_index);
    fs->super_block->used_blocks--;

    return true;
}

bool allocate_block_by_size_for_inode(FileSystem* fs, Inode* inode, size_t size) {
    // 1. 計算需要多少個 blocks
    int blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;  // 向上取整
    
    // 2. 檢查是否有足夠的空間
    int available_blocks = fs->super_block->total_blocks - fs->super_block->used_blocks;
    if (blocks_needed > available_blocks) {
        return false;
    }

    // 保存原始大小，以便在失敗時恢復
    int original_size = inode->size;
    
    // 3. 分配需要的 blocks
    for (int i = 0; i < blocks_needed; i++) {
        int result = allocate_single_block_for_inode(fs, inode);
        if (result == -1) {
            // 分配失敗，釋放已分配的blocks
            inode->size = original_size;  // 恢復原始大小
            free_inode(fs, inode->inode_index);  // 釋放所有已分配的blocks
            
            // 重新分配原有的blocks（如果有的話）
            if (original_size > 0) {
                allocate_block_by_size_for_inode(fs, inode, original_size);
            }
            return false;
        }
    }
    
    // 4. 更新 inode 的大小
    inode->size = size;
    
    return true;
}

void write_block(FileSystem* fs, int block_index, const void* data, size_t size) {
    unsigned char* block_ptr = get_block_position(fs, block_index);
    if (block_ptr != NULL) {
        // 確保不會寫入超過block大小
        size_t write_size = (size > BLOCK_SIZE) ? BLOCK_SIZE : size;
        memcpy(block_ptr, data, write_size);
    }
}

bool write_file_data(FileSystem* fs, Inode* inode, const void* data, size_t size) {
    // 先分配足夠的blocks
    if (!allocate_block_by_size_for_inode(fs, inode, size)) {
        return false;
    }

    // 開始寫入數據
    const char* data_ptr = (const char*)data;
    size_t remaining_size = size;

    // 1. 寫入 direct blocks
    write_direct_block(fs, inode->directBlocks, &data_ptr, &remaining_size);

    // 2. 如果還有數據，寫入 indirect blocks
    if (remaining_size > 0) {
        write_indirect_block(fs, inode->indirectBlock, &data_ptr, &remaining_size);
    }

    return (remaining_size == 0);  // 是否全部寫入完成
}

void write_direct_block(FileSystem* fs, int* directBlocks, const char** data_ptr, size_t* remaining_size) {
    int block_index = 0;
    
    while (*remaining_size > 0 && block_index < BLOCK_NUMBER) {
        if (directBlocks[block_index] != -1) {
            size_t write_size = (*remaining_size > BLOCK_SIZE) ? BLOCK_SIZE : *remaining_size;
            write_block(fs, directBlocks[block_index], *data_ptr, write_size);
            *data_ptr += write_size;      // 更新指向的指標位置
            *remaining_size -= write_size;
        }
        block_index++;
    }
}

void write_indirect_block(FileSystem* fs, int* indirectBlock, const char** data_ptr, size_t* remaining_size) {
    int block_index = 0;
    
    while (*remaining_size > 0 && block_index < BLOCK_NUMBER) {
        if (indirectBlock[block_index] != -1) {
            // 取得這個 indirect block 指向的位置
            int* indirect_block_ptr = (int*)get_block_position(fs, indirectBlock[block_index]);
            // 用 write_direct_block 來寫入這個 indirect block 指向的數據
            write_direct_block(fs, indirect_block_ptr, data_ptr, remaining_size);
        }
        block_index++;
    }
}

void read_block(FileSystem* fs, int block_index, void* buffer, size_t size) {
    unsigned char* block_ptr = get_block_position(fs, block_index);
    if (block_ptr != NULL) {
        size_t read_size = (size > BLOCK_SIZE) ? BLOCK_SIZE : size;
        memcpy(buffer, block_ptr, read_size);
    }
}

size_t read_file_data(FileSystem* fs, Inode* inode, void* buffer) {
    char* buf_ptr = (char*)buffer;
    size_t remaining_size = inode->size;
    size_t total_read = 0;

    // 1. 從 direct blocks 讀取
    read_direct_block(fs, inode->directBlocks, &buf_ptr, &remaining_size, &total_read);

    // 2. 從 indirect blocks 讀取（如果還需要）
    if (remaining_size > 0) {
        read_indirect_block(fs, inode->indirectBlock, &buf_ptr, &remaining_size, &total_read);
    }

    return total_read;
}

void read_direct_block(FileSystem* fs, int* directBlocks, char** buf_ptr, size_t* remaining_size, size_t* total_read) {
    int block_index = 0;
    
    while (*remaining_size > 0 && block_index < BLOCK_NUMBER) {
        if (directBlocks[block_index] != -1) {
            size_t read_size = (*remaining_size > BLOCK_SIZE) ? BLOCK_SIZE : *remaining_size;
            read_block(fs, directBlocks[block_index], *buf_ptr, read_size);
            *buf_ptr += read_size;
            *remaining_size -= read_size;
            *total_read += read_size;
        }
        block_index++;
    }
}

void read_indirect_block(FileSystem* fs, int* indirectBlock, char** buf_ptr, size_t* remaining_size, size_t* total_read) {
    int block_index = 0;
    
    while (*remaining_size > 0 && block_index < BLOCK_NUMBER) {
        if (indirectBlock[block_index] != -1) {
            int* indirect_block_ptr = (int*)get_block_position(fs, indirectBlock[block_index]);
            read_direct_block(fs, indirect_block_ptr, buf_ptr, remaining_size, total_read);
        }
        block_index++;
    }
}

FileSystem* load_filesystem(const unsigned char* data) {
    // 1. 分配新的文件系統結構
    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if (!fs) {
        return NULL;
    }

    // 2. 分配原始空間
    fs->raw_space = data;

    // 3. 設置其他指針
    int metadata_blocks = fs->super_block->system_blocks - fs->super_block->total_inodes;
    fs->super_block = (SuperBlock*)fs->raw_space;
    fs->inode_bitmap = fs->raw_space + sizeof(SuperBlock);
    fs->data_bitmap = fs->inode_bitmap + (fs->super_block->total_inodes + 7) / 8;
    fs->inode_table = (Inode*)(fs->raw_space + (metadata_blocks * BLOCK_SIZE));
    fs->data_blocks = fs->raw_space + (fs->super_block->system_blocks * BLOCK_SIZE);

    return fs;
}

void destroy_space(FileSystem* fs) {
    if (fs) {
        if (fs->raw_space) {
            free(fs->raw_space);
        }
        free(fs);
    }
}
