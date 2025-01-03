#include <stdio.h>
#include <stdbool.h>
#include "inode.h"
#include "space.h"

int main() {
    int main_options, partition_size;
    unsigned char *file_system;

    printf("options:\n");
    printf(" 1. loads from file\n");
    printf(" 2. create new partition in memory\n");
    scanf("%d", &main_options);
    if(main_options == 1) {
        //給明憲做dumpfile的讀寫
        //call真的作業系統func
    } else if(main_options == 2) {
        //分配空間
        printf("Input size of a new partition (example 102400)\n");
        scanf("%d", &partition_size);
        //加一點輸入檢查 至少要超過某個最小值 不要超過某個最大值 取整
        printf("partition size = %d\n", partition_size);
        FileSystem *file_system; 
        file_system = init_space(partition_size);//call初始化func 分配空間、建立特殊資訊、分配node
        //建立Root 


        //call真的作業系統func
    } else {
        printf('input error\n');
        return;
    }

}

int get_command_code(const char *input) {//指令判斷
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

void print_command() {//列出指令
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
    printf('\n');
}

int Interaction(FileSystem *file_system) {
    bool loop_flag = true;
    char input[256];
    //首先初始化一個Inode current_path指向root
    Inode *current_path;
    current_path = 0;//root 也就是buffer的address+block size

    while (loop_flag) {
        print_command();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input\n");//沒東西
            continue;
        }
        input[strcspn(input, "\n")] = '\0';//去掉輸入末尾的換行符    
        char *command = strtok(input, " ");//分割命令和參數
        char *arg = strtok(NULL, " ");

        int command_code = get_command_code(command);

        switch (command_code) {
            case 1:
                //ls();
                //檢查current_path對應的Directory 把KEY全部列出
                break;
            case 2://要考慮絕對路徑 
                //cd();
                //檢查arg 可能要繼續分割
                //新的Inode cd_path 為 current_path
                //loop 檢查cd_path對應的Directory 設定cd_path 為arg對應的Inode 如果有任何錯就直接報錯跳出
                //沒錯的話設定current_path為最終cd_path
                break;
            case 3://要考慮絕對路徑
                //rm();
                //檢查current_path對應的Directory 如果有找到 釋放inode空間、釋放block空間、刪除那組Directory 
                break;
            case 4://要考慮絕對路徑 要考慮多層路徑
                //mkdir();
                //新增一個inode 
                //current_path的Directory 加一條 指向inode
                //新inode的Directory 加兩條 指向自己和current_path
                break;
            case 5://只刪除空目錄 //要考慮絕對路徑 //不能刪除當前目錄
                //rmdir();
                //檢查current_path的Directory 如果有找到 而且對方Directory只有.和.. 釋放inode空間、釋放block空間、刪除那組Directory 
                break;
            case 6:
                //put();
                //看dump資料夾有沒有這東西 有的話分配inode 分配block 掛到current_path的Directory裡
                break;
            case 7://要考慮絕對路徑?
                //get();
                //檢查current_path的Directory 如果有找到 丟到dump
                break;
            case 8://要考慮絕對路徑?
                //cat();
                //檢查current_path的Directory 如果有找到print出內容
                break;
            case 9://要考慮絕對路徑?
                //edit();
                //檢查current_path的Directory 如果有找到 調用珞昱的方法
                break;
            case 10:
                status(file_system->super_block);
                //列出當前超級block內容
                break;
            case 11:
                //help();
                //單純print 一堆字
                break;
            case 12:
                //存檔 明憲做
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
    printf("files blocks:%d\n",status->files_blocks);
    printf("block size:%d\n",status->block_size);
    printf("free space:%d\n",status->free_space);
}