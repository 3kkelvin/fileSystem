#include "space.h"
#include "inode.h"
//初始化一個root
void init_root(FileSystem* fs) {
    //檢查現在是否已經有root了

    int root_index = allocate_inode(fs);
    //那我有index之後怎麼寫東西進inode 應該是回一個地址 然後那個inode裡面已經存了index?

    DirectoryEntry root_directory = { ".", root_index };
    int root_directory_index = allocate_block(fs);
    //然後我怎麼把root_directory寫進root_directory_index 




}