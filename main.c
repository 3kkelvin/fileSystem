#include "main.h"
#include "dump.h"
#include "inode.h"
#include "space.h"


int main() {
    int main_options, partition_size;
    FileSystem *file_system;

    printf("options:\n");
    printf(" 1. loads from file\n");
    printf(" 2. create new partition in memory\n");
    scanf("%d", &main_options);
    if(main_options == 1) {//讀取檔案
        file_system = (FileSystem *)read_dump();
        //todo:需要一個指標重新定位的方法
        if (strcmp(file_system->super_block->password, "mmslab406") != 0) {
            printf("密碼錯誤\n");
        } else {
            printf("密碼正確\n");
            Interaction(file_system);
        }
    } else if(main_options == 2) {//分配空間
        while (1) {//輸入大小
            printf("Input size of a new partition (example 2048000)\n");
            scanf("%d", &partition_size);// 檢查大小範圍：最小 102400，最大 1GB
            if (partition_size < 102400) {
                printf("Error: Partition size must be at least 102400 bytes.\n");
            } else if (partition_size > (1024 * 1024 * 1024)) {
                printf("Error: Partition size must not exceed 1GB.\n");
            } else {           
                partition_size = (partition_size / 1024) * 1024;// 如果不是 1024 的倍數，向下取整
                printf("partition size = %d bytes\n", partition_size);
                break;
            }
        }
        file_system = init_space(partition_size);//call初始化func 分配空間、建立特殊資訊、分配node
        //todo:建立Root 
        
        Interaction(file_system);
    } else {
        printf("input error\n");
        return 0;
    }

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

int Interaction(FileSystem *file_system) {
    bool loop_flag = true;
    char input[256];
    //首先初始化一個Inode current_path指向root
    Inode *current_path;
    current_path = 0;//root 也就是buffer的address+block size
    print_command();
    while (getchar() != '\n' && getchar() != EOF); //確保輸入區沒有髒資料
    while (loop_flag) {
        printf("$ ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input\n");//沒東西
            continue;
        }
        printf("input:%s\n",input);//todo:刪掉
        input[strcspn(input, "\n")] = '\0';//去掉輸入末尾的換行符    
        char *command = strtok(input, " ");//分割命令和參數
        char *arg = strtok(NULL, " ");
        printf("command:%s\n",command);//todo:刪掉
        printf("arg:%s\n",arg);//todo:刪掉
        int command_code = get_command_code(command);

        switch (command_code) {
            case CMD_LS:
                //ls();
                //檢查current_path對應的Directory 把KEY全部列出
                break;
            case CMD_CD://要考慮絕對路徑 
                //cd();
                //檢查arg 可能要繼續分割
                //新的Inode cd_path 為 current_path
                //loop 檢查cd_path對應的Directory 設定cd_path 為arg對應的Inode 如果有任何錯就直接報錯跳出
                //沒錯的話設定current_path為最終cd_path
                break;
            case CMD_RM://要考慮絕對路徑
                //rm();
                //檢查current_path對應的Directory 如果有找到 釋放inode空間、釋放block空間、刪除那組Directory 
                break;
            case CMD_MKDIR://要考慮絕對路徑 要考慮多層路徑
                //mkdir();
                //新增一個inode 
                //current_path的Directory 加一條 指向inode
                //新inode的Directory 加兩條 指向自己和current_path
                break;
            case CMD_RMDIR://只刪除空目錄 //要考慮絕對路徑 //不能刪除當前目錄
                //rmdir();
                //檢查current_path的Directory 如果有找到 而且對方Directory只有.和.. 釋放inode空間、釋放block空間、刪除那組Directory 
                break;
            case CMD_PUT:
                //put();
                //看dump資料夾有沒有這東西 有的話分配inode 分配block 掛到current_path的Directory裡
                break;
            case CMD_GET://要考慮絕對路徑?
                //get();
                //檢查current_path的Directory 如果有找到 丟到dump
                break;
            case CMD_CAT://要考慮絕對路徑?
                //cat();
                //檢查current_path的Directory 如果有找到print出內容
                break;
            case CMD_EDIT://要考慮絕對路徑?
                //edit();
                //檢查current_path的Directory 如果有找到 調用珞昱的方法
                break;
            case CMD_STATUS://列出當前fs資訊
                status(file_system->super_block);
                break;
            case CMD_HELP:
                //help();
                //單純print 一堆字
                break;
            case CMD_EXIT: //存檔
                create_dump(file_system);
                //destroy_space(file_system);
                loop_flag = false;
                break;
            default:
                printf("Unknown command: %s\n", *command);
                break;
        }
    }

    return 0;
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