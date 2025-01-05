#include "command.h"
#include "inode.h"
#include "space.h"

void print_command(void) {
    printf("List of commands\n");
    printf("'ls' list directory\n");
    printf("'cd' change directory\n");
    printf("'rm' remove\n");
    printf("'mkdir' make directory\n");
    printf("'rmdir' remove directory\n");
    printf("'put' put file into the space\n");
    printf("'get' get file from the space\n");
    printf("'cat' show content\n");
    printf("'edit' edit file with vim\n");
    printf("'status' show status of space\n");
    printf("'help' \n");
    printf("'exit' exit and store img\n");
    printf("\n");
}

int get_command_code(const char *input) {
    if (strcmp(input, "ls") == 0) return 1;
    if (strcmp(input, "cd") == 0) return 2;
    if (strcmp(input, "rm") == 0) return 3;
    if (strcmp(input, "mkdir") == 0) return 4;
    if (strcmp(input, "rmdir") == 0) return 5;
    if (strcmp(input, "put") == 0) return 6;
    if (strcmp(input, "get") == 0) return 7;
    if (strcmp(input, "cat") == 0) return 8;
    if (strcmp(input, "edit") == 0) return 9;
    if (strcmp(input, "status") == 0) return 10;
    if (strcmp(input, "help") == 0) return 11;
    if (strcmp(input, "exit") == 0) return 12;
    return 0; // Unknown command
}

void status(SuperBlock *status) {
    printf("partition size:%d\n",status->partition_size);
    printf("total inodes:%d\n",status->total_inodes);
    printf("used inodes:%d\n",status->used_inodes);
    printf("total blocks:%d\n",status->total_blocks);
    printf("used blocks:%d\n",status->used_blocks);
    //printf("files blocks:%d\n",status->total_data);
    printf("block size:%d\n",status->block_size);
    //printf("free space:%d\n",status->free_space);
    printf("\n\n");
}

void ls(FileSystem* fs, Inode* inode) {
    printf("ls:");
    for (int i = 0; i < BLOCK_NUMBER; ++i) {
        if (inode->directBlocks[i] == -1) {
            break;
        }
        read_directory_entries(fs, inode->directBlocks[i]);
    }
    //todo:處理多重block
    printf("\n");
}
//讀取一個block內的所有檔案/路徑名並print出
void read_directory_entries(FileSystem* fs, int block_index) {
    unsigned char* block_address = get_block_position(fs, block_index);
    int offset = 0;
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    while (offset + DirectoryEntrySize <= BLOCK_SIZE) {//如果空間還夠 檢查下一組key-value位址  
        DirectoryEntry* entry = (DirectoryEntry*)(block_address + offset);
        bool self_or_father = (strcmp(entry->filename, ".") == 0 || strcmp(entry->filename, "..") == 0);//自己或父路徑 有可能指向root(0) 所以要建立特例
        if (!self_or_father && entry->inode_index == 0) {//如果為空跳出
            break;
        } 
        printf("%s ", entry->filename);//輸出檔案名 todo:屏蔽已刪除或是. ..
        offset += DirectoryEntrySize;//不為空 指向下一組
    }
}

Inode* cd(FileSystem* fs, Inode* inode, char *arg) {
    Inode *temp_inode = (Inode *)malloc(sizeof(Inode));
    memcpy(temp_inode, inode, sizeof(Inode));//複製inode
    size_t length = strlen(arg)+1;
    char temp_arg[length];
    strncpy(temp_arg, arg, length);//複製路徑到字串陣列
    char* token = strtok(temp_arg, "/");//切割陣列

    if (arg[0] == '/') {//絕對路徑
        if (strcmp(token, "root") != 0) {
            printf("錯誤的絕對路徑");
            return inode;
        }
        temp_inode = get_inode(fs, 0);//root
        token = strtok(NULL, "/");//第二段路徑  
    }
    //尋找已存在的路徑
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    bool keep_find = true;
    while (token != NULL) {
        bool found_next_path = false;
        for (int i = 0; i < BLOCK_NUMBER; ++i) {
            if (found_next_path) {//找到下一層路徑
                break;
            }
            if (temp_inode->directBlocks[i] == -1) {//已找完當前路徑下的所有子路徑
                keep_find = false;//找不到 直接跳出while
                break;
            }
            unsigned char* block_address = get_block_position(fs, temp_inode->directBlocks[i]);
            int offset = 0;
            while (offset + DirectoryEntrySize <= BLOCK_SIZE) {//如果空間還夠 檢查下一組key-value位址  
                DirectoryEntry* entry = (DirectoryEntry*)(block_address + offset);
                bool self_or_father = (strcmp(entry->filename, ".") == 0 || strcmp(entry->filename, "..") == 0);//自己或父路徑 有可能指向root(0) 所以要建立特例
                if (!self_or_father && entry->inode_index == 0) {//如果為空跳出
                    break;
                } 
                if (strcmp(token, entry->filename) == 0) {//找到同名路徑
                    found_next_path = true;
                    temp_inode = get_inode(fs, entry->inode_index);//指向下一層
                    break;
                }
                offset += DirectoryEntrySize;//指向下一組
            }
    
        }
        if (!keep_find) {
            break;//沒找到
        }
        token = strtok(NULL, "/");
    }
    if(token == NULL) {//cd的目的地真的存在
        return temp_inode;
    }
    return inode;//路徑不完全存在
}

void mkdir(FileSystem* fs, Inode *inode, char* arg) {
    Inode *temp_inode = (Inode *)malloc(sizeof(Inode));
    memcpy(temp_inode, inode, sizeof(Inode));//複製inode
    size_t length = strlen(arg)+1;
    char temp_arg[length];
    strncpy(temp_arg, arg, length);//複製路徑到字串陣列
    char* token = strtok(temp_arg, "/");//切割陣列
    
    if (arg[0] == '/') {//絕對路徑
        if (strcmp(token, "root") != 0) {
            printf("錯誤的絕對路徑");
            return;
        }
        temp_inode = get_inode(fs, 0);//root
        token = strtok(NULL, "/");//第二段路徑  
    }
    //尋找已存在的路徑
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    bool keep_find = true;
    while (token != NULL) {
        bool found_next_path = false;
        for (int i = 0; i < BLOCK_NUMBER; ++i) {
            if (found_next_path) {//找到下一層路徑
                break;
            }
            if (temp_inode->directBlocks[i] == -1) {//已找完當前路徑下的所有子路徑
                keep_find = false;//找不到 直接跳出while去新增路徑路徑
                break;
            }
            unsigned char* block_address = get_block_position(fs, temp_inode->directBlocks[i]);
            int offset = 0;
            while (offset + DirectoryEntrySize <= BLOCK_SIZE) {//如果空間還夠 檢查下一組key-value位址  
                DirectoryEntry* entry = (DirectoryEntry*)(block_address + offset);
                bool self_or_father = (strcmp(entry->filename, ".") == 0 || strcmp(entry->filename, "..") == 0);//自己或父路徑 有可能指向root(0) 所以要建立特例
                if (!self_or_father && entry->inode_index == 0) {//如果為空跳出
                    break;
                }        
                if (strcmp(token, entry->filename) == 0) {//找到同名路徑
                    found_next_path = true;
                    temp_inode = get_inode(fs, entry->inode_index);//指向下一層
                    break;
                }
                offset += DirectoryEntrySize;//指向下一組
            }
        }
        if (!keep_find) {
            break;//沒找到 直接去新增路徑
        }
        token = strtok(NULL, "/");
    }
    //將剩餘的路徑建立資料夾
    while (token != NULL) {
        //新增一個inode 
        Inode *new_inode; 
        new_inode = allocate_inode(fs, false);//新建inode
        int block_index = allocate_single_block_for_inode(fs, new_inode);//分配block
        DirectoryEntry self_directory = { ".", new_inode->inode_index};//新建指向自己的key-value
        DirectoryEntry father_directory = { "..", temp_inode->inode_index};//新建指向父資料夾的key-value
        write_directory_entry(fs, new_inode, block_index, &self_directory);
        write_directory_entry(fs, new_inode, block_index, &father_directory);

        DirectoryEntry new_directory;//父資料夾指向新資料夾
        strncpy(new_directory.filename, token, MAX_FILENAME_LENGTH - 1);
        new_directory.inode_index = new_inode->inode_index;   
        for (int i = 0; i < BLOCK_NUMBER; ++i) {
            if (temp_inode->directBlocks[i] == -1) {
                write_directory_entry(fs, temp_inode, temp_inode->directBlocks[i-1], &new_directory);//寫入父資料夾
                break;
            }
        }
        temp_inode = new_inode;//指向下一層
        token = strtok(NULL, "/");
    }
}

