#include "dump.h"
#include "space.h"

// 主程序 - dump 功能
bool create_dump(FileSystem *fs) {
    char filename[256] = "my_fs.dump";
    char password[PASSWORD_SIZE];

    // 輸入密碼
    printf("請設定加密密碼：");
    if (fgets(password, PASSWORD_SIZE, stdin) == NULL) {
        perror("輸入密碼失敗");
        return false;
    }
    password[strcspn(password, "\n")] = '\0';

    // 存入文件
    return save_to_dumpfile(fs, filename, password);
}

bool save_to_dumpfile(FileSystem* fs, const char* filename, const char *password) {
    // 打開dump文件
    FILE* dump_file = fopen(filename, "wb");
    if (dump_file == NULL) {
        perror("無法創建 dump file");
        return false;
    }

    unsigned char *encrypted_data = (unsigned char *)malloc(fs->super_block->partition_size);
    if (!encrypted_data) {
        perror("加密空間分配失敗");
        fclose(dump_file);
        return false;
    }
    memcpy(encrypted_data, fs->raw_space, fs->super_block->partition_size);
    xor_encrypt_decrypt(encrypted_data, fs->super_block->partition_size, password);
    
    // 寫入加密數據
    if (fwrite(encrypted_data, 1, fs->super_block->partition_size, dump_file) != fs->super_block->partition_size) {
        perror("寫入加密數據失敗");
        free(encrypted_data);
        fclose(dump_file);
        return false;
    }

    // 關閉文件
    free(encrypted_data);
    fclose(dump_file);
    
    // 檢查是否完整寫入
    return true;
}

// XOR 加密與解密
void xor_encrypt_decrypt(unsigned char *data, size_t size, const char *password) {
    size_t pass_len = strlen(password);
    for (size_t i = 0; i < size; i++) {
        data[i] ^= password[i % pass_len];
    }
}

// 主程序 - 讀取功能
FileSystem *read_dump() {
    char filename[256] = "my_fs.dump";
    char password[PASSWORD_SIZE];

    // 輸入密碼
    printf("請輸入解密密碼：");
    if (fgets(password, PASSWORD_SIZE, stdin) == NULL) {
        perror("輸入密碼失敗");
        return NULL;
    }
    password[strcspn(password, "\n")] = '\0';

    unsigned char *decrypted_data = load_memory(password);
    if (!decrypted_data) {
        return NULL;
    }

    //檢查是否解碼成功
    SuperBlock *sb = (SuperBlock *)decrypted_data;
    if (strcmp(sb->password, "mmslab406") != 0) {
        printf("密碼錯誤\n");
        free(decrypted_data);
        return NULL;
    }

    // 讀取並解密
    return load_filesystem(decrypted_data);
}

// 從 dump file 中讀取並解密內容
unsigned char *load_memory(const char *password) {
    FILE *file = fopen("my_fs.dump", "rb");
    if (!file) {
        perror("無法打開 dump file");
        return NULL;
    }

    // 獲取文件大小並讀取加密數據
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    unsigned char *encrypted_data = (unsigned char *)malloc(file_size);
    if (!encrypted_data) {
        perror("記憶體分配失敗");
        fclose(file);
        return NULL;
    }

    if (fread(encrypted_data, 1, file_size, file) != file_size) {
        perror("讀取加密數據失敗");
        free(encrypted_data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    // 解密數據
    xor_encrypt_decrypt(encrypted_data, file_size, password);
    
    return encrypted_data;
}
