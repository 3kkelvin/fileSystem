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
        printf("%s ", entry->filename);//輸出檔案名
        offset += DirectoryEntrySize;//不為空 指向下一組
    }
}