#include "main.h"
#include "dump.h"
#include "inode.h"
#include "space.h"
#include "command.h"

int main() {
    int main_options, partition_size;
    FileSystem *file_system;

    printf("options:\n");
    printf(" 1. loads from file\n");
    printf(" 2. create new partition in memory\n");
    scanf("%d", &main_options);
    if(main_options == 1) {//讀取檔案
        file_system = (FileSystem *)read_dump();
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
        file_system = init_space(partition_size);
        init_root(file_system);//建立root
        Interaction(file_system);
    } else {
        printf("input error\n");
        return 0;
    }

}

int Interaction(FileSystem *file_system) {
    bool loop_flag = true;
    char input[256];
    char current_path_text[256];
    //首先初始化一個Inode current_path指向root
    Inode *current_path;
    current_path = get_inode(file_system, 0);//root  
    print_command();
    while (getchar() != '\n' && getchar() != EOF); //確保輸入區沒有髒資料
    while (loop_flag) {
        printf("%s/ $ ",current_path_text);
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
                ls(file_system, current_path);
                break;
            case CMD_CD:
                current_path = cd(file_system, current_path, arg, current_path_text);
                break;
            case CMD_RM:
                rm(file_system, current_path, arg);
                break;
            case CMD_MKDIR:
                if (arg == NULL) {//沒路徑
                    break;
                }
                my_mkdir(file_system, current_path, arg);
                break;
            case CMD_RMDIR:
                if (arg == NULL) {//沒路徑
                    break;
                }
                my_rmdir(file_system, current_path, arg);
                break;
            case CMD_PUT:
                if (arg == NULL) {//沒檔案
                    break;
                }
                put(file_system, current_path, arg);
                break;
            case CMD_GET:
                if (arg == NULL) {//沒檔案
                    break;
                }
                get(file_system, current_path, arg);
                break;
            case CMD_CAT:
                if (arg == NULL) {//沒檔案
                    break;
                }
                cat(file_system, current_path, arg);
                //檢查current_path的Directory 如果有找到print出內容
                break;
            case CMD_CREATE://要考慮絕對路徑?
                my_create(file_system, current_path, arg);
                //檢查current_path的Directory 如果有找到 調用珞昱的方法
                break;
            case CMD_EDIT://要考慮絕對路徑?
                edit(file_system, current_path, arg);
                //檢查current_path的Directory 如果有找到 調用珞昱的方法
                break;
            case CMD_STATUS://列出當前fs資訊
                status(file_system->super_block);
                break;
            case CMD_HELP:
                print_command();//重新print出能用的指令
                break;
            case CMD_EXIT: //存檔
                create_dump(file_system);
                //destroy_space(file_system);
                loop_flag = false;
                break;
            default:
                printf("Unknown command\n");
                break;
        }
    }

    return 0;
}

