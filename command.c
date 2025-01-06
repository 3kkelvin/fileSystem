#include "command.h"
#include "inode.h"
#include "space.h"
#include "edit.h"

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
        if (entry->inode_index > -1)
        {
            printf("%s ", entry->filename);//輸出檔案名 todo:屏蔽. ..
        }
        offset += DirectoryEntrySize;//不為空 指向下一組
    }
}

Inode* cd(FileSystem* fs, Inode* inode, char *arg, char *text) {
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
        if (arg[0] == '/') {//絕對路徑
            strcpy(text, arg);//直接複製
            if (strncmp(text, "/root", 5) == 0) {
                memmove(text, text + 5, strlen(text + 5) + 1); //再移除前面的/root
            }
            strcpy(text, arg);
        } else {//相對路徑
            if(strcmp(arg, ".") == 0) {//cd自己
            } else if (strcmp(arg, "..") == 0) {//cd上層
                char *last_slash = strrchr(text, '/'); //移除最後一段/xxx
                if (last_slash != NULL) {
                    *last_slash = '\0';
                }
            } else {
                strcat(text, "/");//加入/
                strcat(text, arg); //加入arg到後面
            }
        }
        return temp_inode;
    }
    return inode;//路徑不完全存在
}

void rm(FileSystem* fs, Inode *inode, char* arg) {
    Inode *temp_inode = (Inode *)malloc(sizeof(Inode));
    memcpy(temp_inode, inode, sizeof(Inode));//複製inode
    size_t length = strlen(arg)+1;
    char temp_arg[length];
    strncpy(temp_arg, arg, length);//複製路徑到字串陣列
    char* token = strtok(temp_arg, "/");//切割陣列
    //取得檔案名
    char file_name[MAX_FILENAME_LENGTH];
    const char* last_slash = strrchr(arg, '/');
    if (strrchr(arg, '/') != NULL) {
        strcpy(file_name, last_slash + 1);
    } else {
        strcpy(file_name, arg);
    }

    if (arg[0] == '/') {//絕對路徑
        if (strcmp(token, "root") != 0) {
            printf("錯誤的絕對路徑");
            return;
        }
        temp_inode = get_inode(fs, 0);//root
        token = strtok(NULL, "/");//第二段路徑
        if (token == NULL) {
            printf("不能刪除root");
            return;
        }  
    }
    //尋找已存在的路徑
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    bool keep_find = true;
    int file_index;
    while (token != NULL) {
        if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0 ) {
            printf("不可刪除自己或父路徑");
            return;
        }
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
                if (strcmp(file_name, entry->filename) == 0) {//找到同名檔案
                    found_next_path = true;   
                    file_index = entry->inode_index;
                    break;
                }
                if (strcmp(token, entry->filename) == 0) {//找到同名資料夾
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
    
    Inode* file_inode = get_inode(fs, file_index);
    if(token == NULL) {//rm的目的地真的存在
        if(file_inode->isFile != true) {//如果不是檔案
            return;
        }
        //執行刪除 釋放inode空間、釋放block空間、刪除父路徑那組Directory
        bool is_delete = false;
        for (int i = 0; i < BLOCK_NUMBER; ++i) {//從父資料夾移除該資料
            if (temp_inode->directBlocks[i] == -1) {//已找完當前路徑下的所有子路徑
                break;
            }
            unsigned char* block_address = get_block_position(fs, temp_inode->directBlocks[i]);
            int offset = 0;
            while (offset + DirectoryEntrySize <= BLOCK_SIZE) {//如果空間還夠 檢查下一組key-value位址  
                DirectoryEntry* entry = (DirectoryEntry*)(block_address + offset);
                if (entry->inode_index == file_inode->inode_index) {//找到子資料夾 刪除路徑
                    entry->inode_index = -1;
                    memset(entry->filename, 0, sizeof(entry->filename));
                    is_delete = true;
                    break;
                } 
                offset += DirectoryEntrySize;//指向下一組 
            }
        }
        if(!is_delete) {
            printf("刪除失敗:父資料夾");
            return;
        }
        is_delete = free_inode(fs, temp_inode->inode_index);//釋放檔案inode
        if(!is_delete) {
            printf("刪除失敗:檔案");
            return;
        }
    }
    return;
}

void my_mkdir(FileSystem* fs, Inode *inode, char* arg) {
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
        DirectoryEntry self_directory = {".", new_inode->inode_index};//新建指向自己的key-value
        DirectoryEntry father_directory = {"..", temp_inode->inode_index};//新建指向父資料夾的key-value
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

void my_rmdir(FileSystem* fs, Inode *inode, char* arg) {
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
        if (token == NULL) {
            printf("不能刪除root");
            return;
        }  
    }
    
    //尋找已存在的路徑
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    bool keep_find = true;
    while (token != NULL) {
        if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0 ) {
            printf("不可刪除自己或父路徑");
            return;
        }
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

    if(token == NULL) {//rmdir的目的地真的存在
        if(temp_inode->isFile != false) {//如果不是資料夾
            return;
        }
        for (int i = 0; i < BLOCK_NUMBER; ++i) {//檢查末端資料夾是否為空
            if (temp_inode->directBlocks[i] == -1) {//已找完當前路徑下的所有子路徑
                break;//找不到任何檔案/路徑
            }
            unsigned char* block_address = get_block_position(fs, temp_inode->directBlocks[i]);
            int offset = 0;
            while (offset + DirectoryEntrySize <= BLOCK_SIZE) {//如果空間還夠 檢查下一組key-value位址
                DirectoryEntry* entry = (DirectoryEntry*)(block_address + offset);
                bool self_or_father = (strcmp(entry->filename, ".") == 0 || strcmp(entry->filename, "..") == 0);//自己或父路徑 不算數
                if (!self_or_father && entry->inode_index > 0) {//如果有不是空的路徑就跳出
                    return;
                }
                offset += DirectoryEntrySize;//指向下一組 
            }
        }
        //執行刪除 釋放inode空間、釋放block空間、刪除父路徑那組Directory
        bool is_delete = false;

        DirectoryEntry* father_entry = (DirectoryEntry*)(get_block_position(fs, temp_inode->directBlocks[0]) + DirectoryEntrySize);
        int father_inode_index = father_entry->inode_index;
        Inode* father = get_inode(fs, father_inode_index);
        for (int i = 0; i < BLOCK_NUMBER; ++i) {//從父資料夾移除該資料
            if (father->directBlocks[i] == -1) {//已找完當前路徑下的所有子路徑
                break;
            }
            unsigned char* block_address = get_block_position(fs, father->directBlocks[i]);
            int offset = 0;
            while (offset + DirectoryEntrySize <= BLOCK_SIZE) {//如果空間還夠 檢查下一組key-value位址  
                DirectoryEntry* entry = (DirectoryEntry*)(block_address + offset);
                if (entry->inode_index == temp_inode->inode_index) {//找到子資料夾 刪除路徑
                    entry->inode_index = -1;
                    memset(entry->filename, 0, sizeof(entry->filename));
                    is_delete = true;
                    break;
                } 
                offset += DirectoryEntrySize;//指向下一組 
            }
        }
        if(!is_delete) {
            printf("刪除失敗:父資料夾");
            return;
        }
        is_delete = free_inode(fs, temp_inode->inode_index);//釋放子資料夾inode
        if(!is_delete) {
            printf("刪除失敗:子資料夾");
            return;
        }
    }
    return;
}

void put(FileSystem* fs, Inode *inode, char *arg) {
    char filePath[256] = {'\0'};//目標檔案的真實路徑="file_system/"+arg
    strcat(filePath, "file_system/");
    strcat(filePath, arg);
    struct stat buffer;
    if (stat(filePath, &buffer) != 0) {// 確認檔案存在
        printf("檔案不存在:%s",filePath);
        return;
    }
    FILE *file = fopen(filePath, "rb");// 打開檔案
    if (!file) {
        printf("檔案打不開");
        return;
    }
    size_t fileSize = buffer.st_size;//取得檔案大小
    
    void *fileData = malloc(fileSize);
    if (!fileData) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return;
    }
    fread(fileData, 1, fileSize, file);// 讀取檔案內容
    fclose(file);
    
    Inode* new_inode;
    new_inode = allocate_inode(fs, true);//新建inode
    new_inode->size = fileSize;
    if(!write_file_data(fs, new_inode, fileData, fileSize)) {
        printf("寫入失敗");
        return;
    }
    DirectoryEntry new_directory;//父資料夾指向新檔案
    strncpy(new_directory.filename, arg, MAX_FILENAME_LENGTH - 1);
    new_directory.inode_index = new_inode->inode_index;  
    for (int i = 0; i < BLOCK_NUMBER; ++i) {
        if (inode->directBlocks[i] == -1) {
            write_directory_entry(fs, inode, inode->directBlocks[i-1], &new_directory);//寫入父資料夾
            break;
        }
    }
    free(fileData);
    printf("File %s successfully added to the file system.\n", arg);
}

void get(FileSystem* fs, Inode *inode, char *arg) {
    Inode *temp_inode = (Inode *)malloc(sizeof(Inode));
    memcpy(temp_inode, inode, sizeof(Inode));//複製inode
    size_t length = strlen(arg)+1;
    char temp_arg[length];
    strncpy(temp_arg, arg, length);//複製路徑到字串陣列
    char* token = strtok(temp_arg, "/");//切割陣列
    //取得檔案名
    char file_name[MAX_FILENAME_LENGTH];
    const char* last_slash = strrchr(arg, '/');
    if (strrchr(arg, '/') != NULL) {
        strcpy(file_name, last_slash + 1);
    } else {
        strcpy(file_name, arg);
    }

    if (arg[0] == '/') {//絕對路徑
        if (strcmp(token, "root") != 0) {
            printf("錯誤的絕對路徑");
            return;
        }
        temp_inode = get_inode(fs, 0);//root
        token = strtok(NULL, "/");//第二段路徑
        if (token == NULL) {
            printf("缺少檔案名稱");
            return;
        }  
    }
    //尋找已存在的路徑
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    bool keep_find = true;
    int file_index;
    while (token != NULL) {
        if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0 ) {
            printf("錯誤的路徑");
            return;
        }
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
                if (strcmp(file_name, entry->filename) == 0) {//找到同名檔案
                    found_next_path = true;   
                    file_index = entry->inode_index;
                    break;
                }
                if (strcmp(token, entry->filename) == 0) {//找到同名資料夾
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
    //讀取資料
    Inode* file_inode;
    file_inode = get_inode(fs, file_index);
    size_t file_size = file_inode->size; 
    void* buffer = malloc(file_size);
    if (!buffer) {
        printf("開不了buffer\n");
        return;
    }
    if (!read_file_data(fs, file_inode, buffer)) {
        printf("讀取錯誤\n");
        free(buffer);
        return;
    }
    //寫入真實路徑
    char filePath[256] = {'\0'};//目標檔案的真實路徑="file_system/dump/"+arg
    strcat(filePath, "file_system/dump/");
    strcat(filePath, file_name);
    FILE *outputFile = fopen(filePath, "wb");//打開目標檔案
    if (!outputFile) {
        printf("開檔錯誤\n");
        free(buffer);
        return;
    }
    fwrite(buffer, 1, file_size, outputFile);//寫入檔案資料
    fclose(outputFile);
    //清理
    free(buffer);
    printf("File successfully extracted to %s.\n", file_name);
}

void cat(FileSystem* fs, Inode *inode, char *arg) {
    Inode *temp_inode = (Inode *)malloc(sizeof(Inode));
    memcpy(temp_inode, inode, sizeof(Inode));//複製inode
    size_t length = strlen(arg)+1;
    char temp_arg[length];
    strncpy(temp_arg, arg, length);//複製路徑到字串陣列
    char* token = strtok(temp_arg, "/");//切割陣列
    //取得檔案名
    char file_name[MAX_FILENAME_LENGTH];
    const char* last_slash = strrchr(arg, '/');
    if (strrchr(arg, '/') != NULL) {
        strcpy(file_name, last_slash + 1);
    } else {
        strcpy(file_name, arg);
    }

    if (arg[0] == '/') {//絕對路徑
        if (strcmp(token, "root") != 0) {
            printf("錯誤的絕對路徑");
            return;
        }
        temp_inode = get_inode(fs, 0);//root
        token = strtok(NULL, "/");//第二段路徑
        if (token == NULL) {
            printf("缺少檔案名稱");
            return;
        }  
    }
    //尋找已存在的路徑
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    bool keep_find = true;
    int file_index;
    while (token != NULL) {
        if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0 ) {
            printf("錯誤的路徑");
            return;
        }
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
                if (strcmp(file_name, entry->filename) == 0) {//找到同名檔案
                    found_next_path = true;   
                    file_index = entry->inode_index;
                    break;
                }
                if (strcmp(token, entry->filename) == 0) {//找到同名資料夾
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
    //讀取資料
    Inode* file_inode;
    file_inode = get_inode(fs, file_index);
    size_t file_size = file_inode->size; 
    void* buffer = malloc(file_size);
    if (!buffer) {
        printf("開不了buffer\n");
        return;
    }
    if (!read_file_data(fs, file_inode, buffer)) {
        printf("讀取錯誤\n");
        free(buffer);
        return;
    }
    
    fwrite(buffer, 1, file_size, stdout);//印出檔案內容
    printf("\n");

    //清理
    free(buffer);
}

void edit(FileSystem* fs, Inode *inode, char *arg) {
    Inode *temp_inode = (Inode *)malloc(sizeof(Inode));
    memcpy(temp_inode, inode, sizeof(Inode));//複製inode
    size_t length = strlen(arg)+1;
    char temp_arg[length];
    strncpy(temp_arg, arg, length);//複製路徑到字串陣列
    char* token = strtok(temp_arg, "/");//切割陣列
    //取得檔案名
    char file_name[MAX_FILENAME_LENGTH];
    const char* last_slash = strrchr(arg, '/');
    if (strrchr(arg, '/') != NULL) {
        strcpy(file_name, last_slash + 1);
    } else {
        strcpy(file_name, arg);
    }

    if (arg[0] == '/') {//絕對路徑
        if (strcmp(token, "root") != 0) {
            printf("錯誤的絕對路徑");
            return;
        }
        temp_inode = get_inode(fs, 0);//root
        token = strtok(NULL, "/");//第二段路徑
        if (token == NULL) {
            printf("缺少檔案名稱");
            return;
        }  
    }
    //尋找已存在的路徑
    size_t DirectoryEntrySize = sizeof(DirectoryEntry);
    bool keep_find = true;
    int file_index;
    while (token != NULL) {
        if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0 ) {
            printf("錯誤的路徑");
            return;
        }
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
                if (strcmp(file_name, entry->filename) == 0) {//找到同名檔案
                    found_next_path = true;   
                    file_index = entry->inode_index;
                    break;
                }
                if (strcmp(token, entry->filename) == 0) {//找到同名資料夾
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
    //讀取資料
    Inode* file_inode;
    file_inode = get_inode(fs, file_index);
    size_t file_size = file_inode->size; 
    void* buffer = malloc(file_size);
    if (!buffer) {
        printf("開不了buffer\n");
        return;
    }
    if (!read_file_data(fs, file_inode, buffer)) {
        printf("讀取錯誤\n");
        free(buffer);
        return;
    }
    edit_buffer_with_vim((void**)&buffer, &file_size);
    
    fwrite(buffer, 1, file_size, stdout);//印出檔案內容
    printf("\n");

    //清理
    free(buffer);
}