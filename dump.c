#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dump.h"

// 主程序 - dump 功能
void create_dump(FileSystem *fs) {
    char password[PASSWORD_SIZE];

    // 輸入密碼
    printf("請設定加密密碼：");
    if (fgets(password, PASSWORD_SIZE, stdin) == NULL) {
        perror("輸入密碼失敗");
        return;
    }
    password[strcspn(password, "\n")] = '\0';

    // 存入文件
    dump_memory(fs->raw_space, (size_t)(fs->super_block->partition_size), password);
}

// 將記憶體空間內容存入 dump file
void dump_memory(const unsigned char *memory_space, size_t size, const char *password) {
    // 創建檔案
    FILE *file = fopen("my_fs.dump", "wb");
    if (!file) {
        perror("無法創建 dump file");
        exit(EXIT_FAILURE);
    }

    // 加密數據
    unsigned char *encrypted_data = (unsigned char *)malloc(size);
    if (!encrypted_data) {
        perror("加密空間分配失敗");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    memcpy(encrypted_data, memory_space, size);
    xor_encrypt_decrypt(encrypted_data, size, password);

    // 寫入加密數據
    if (fwrite(encrypted_data, 1, size, file) != size) {
        perror("寫入加密數據失敗");
        free(encrypted_data);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    free(encrypted_data);
    fclose(file);
    printf("記憶體內容已加密並存入 %s\n", "my_fs.dump");
}

// XOR 加密與解密
void xor_encrypt_decrypt(unsigned char *data, size_t size, const char *password) {
    size_t pass_len = strlen(password);
    for (size_t i = 0; i < size; i++) {
        data[i] ^= password[i % pass_len];
    }
}

// 主程序 - 讀取功能
unsigned char *read_dump() {
    char password[PASSWORD_SIZE];

    // 輸入密碼
    printf("請輸入解密密碼：");
    if (fgets(password, PASSWORD_SIZE, stdin) == NULL) {
        perror("輸入密碼失敗");
        return;
    }
    password[strcspn(password, "\n")] = '\0';

    // 讀取並解密
    return load_memory(password);
}

// 從 dump file 中讀取並解密內容
unsigned char *load_memory(const char *password) {
    FILE *file = fopen("my_fs.dump", "rb");
    if (!file) {
        perror("無法打開 dump file");
        exit(EXIT_FAILURE);
    }

    // 獲取文件大小並讀取加密數據
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    unsigned char *encrypted_data = (unsigned char *)malloc(file_size);
    if (!encrypted_data) {
        perror("記憶體分配失敗");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(encrypted_data, 1, file_size, file) != file_size) {
        perror("讀取加密數據失敗");
        free(encrypted_data);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // 解密數據
    xor_encrypt_decrypt(encrypted_data, file_size, password);
    
    //在這邊檢查檔案前面加密特徵 確定是否有成功解碼

    return encrypted_data;

    
}



