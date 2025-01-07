#include "edit.h"
void edit_buffer_with_vim(void** buffer, size_t* size) {
    // 創建一個臨時檔案
    char temp_filename[] = "tempfileXXXXXX";
    char temp_filename2[32];
    char temp_filename3[32]; 
    int fd = mkstemp(temp_filename);
    snprintf(temp_filename2, 32, "%s~", temp_filename);
    snprintf(temp_filename3, 32, ".%s.un~", temp_filename);
    if (fd == -1) {
        perror("mkstemp");
        return;
    }
    // 將 buffer 寫入臨時檔案
    FILE* temp_file = fdopen(fd, "wb+");
    if (!temp_file) {
        perror("fdopen");
        close(fd);
        return;
    }
    fwrite(*buffer, 1, *size, temp_file);
    fflush(temp_file);

    // 使用 vim 編輯臨時檔案
    char command[256];
    snprintf(command, sizeof(command), "vim %s", temp_filename);
    system(command);
    // 獲取編輯後檔案的大小
    fseek(temp_file, 0, SEEK_END);
    long new_size = ftell(temp_file);
    fseek(temp_file, 0, SEEK_SET);

    // 如果新大小不等於原來的大小，重新分配緩衝區
    if (new_size != *size) {
        void* new_buffer = realloc(*buffer, new_size);
        if (!new_buffer) {
            printf("重新分配記憶體失敗");
            fclose(temp_file);
            remove(temp_filename);
            return;
        }
        *buffer = new_buffer;
        *size = new_size;
    }

    // 讀回編輯後的內容
    fread(*buffer, 1, new_size, temp_file);
    // 關閉並刪除臨時檔案
    fclose(temp_file);
    remove(temp_filename);
    remove(temp_filename2);
    remove(temp_filename3);
}
