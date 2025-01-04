#include "space.h"
#include "inode.h"
//初始化一個root
void init_root(FileSystem* fs) {
    //todo?檢查現在是否已經有root了?

    Inode *root; 
    root = allocate_inode(fs, false);//新建inode
    int block_index = allocate_single_block_for_inode(fs, root);//分配block
    DirectoryEntry root_directory = { ".", root->inode_index};//新建指向自己的key-value
    write_directory_entry(fs, block_index, &root_directory);

}
//負責寫入key-value 返回到底存在哪個block
int write_directory_entry(FileSystem* fs, int current_block_index, DirectoryEntry* new_entry) {//負責寫入key-value
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);//key-value大小
    unsigned char* block_address = get_block_position(fs, current_block_index);//取得block位址

    size_t offset = 0;
    while (offset + DirectoryEntrySize <= BLOCK_SIZE) {//如果空間還夠 檢查指向的key-value位址  
        DirectoryEntry* existing_entry = (DirectoryEntry*)(block_address + offset);
        if (existing_entry->inode_index == 0) {//如果為空 寫入
            memcpy(existing_entry, new_entry, DirectoryEntrySize);
            return current_block_index;
        }
        offset += DirectoryEntrySize;//不為空 指向下一組
    }
    // 如果block已滿，分配下一個block
    int new_block_index = allocate_data_block(fs);
    block_address = get_block_position(fs, current_block_index);
    memcpy(block_address, new_entry, DirectoryEntrySize);//寫入下一個block

    return new_block_index;
}