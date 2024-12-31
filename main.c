#include <stdio.h>
#include <stdbool.h>
int main() {
    int main_options, partition_size;
    unsigned char *file_system;

    printf("options:\n");
    printf(" 1. loads from file\n");
    printf("  2. create new partition in memory\n");
    scanf("%d", &main_options);
    if(main_options == 1) {
        //給明憲做dumpfile的讀寫
        //call真的作業系統func
    } else if(main_options == 2) {
        //分配空間
        printf("Input size of a new partition (example 102400)\n");
        scanf("%d", &partition_size)
        printf("partition size = %d\n", partition_size);
        //call初始化func 分配空間、建立特殊資訊、分配node
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
    if (strcmp(input, "status") == 0) return 9;
    if (strcmp(input, "help") == 0) return 10;
    if (strcmp(input, "exit") == 0) return 11;
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
    printf("'status' show status of space\n");
    printf("'help' \n");
    printf("'exit' exit and store img\n");
    printf('\n');
}

int Interaction(unsigned char *file_system) {
    bool loop_flag = true;
    char input[256];

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
                break;
            case 2:
                //cd();
                break;
            case 3:
                //rm();
                break;
            case 4:
                //mkdir();
                break;
            case 5:
                //rmdir();
                break;
            case 6:
                //put();
                break;
            case 7:
                //get();
                break;
            case 8:
                //cat();
                break;
            case 9:
                //status();
                break;
            case 10:
                //help();
                break;
            case 11:
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