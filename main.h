#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <stdbool.h>
typedef struct SuperBlock SuperBlock;
typedef struct FileSystem FileSystem;

typedef enum {
    CMD_UNKNOWN = 0, // 未知命令
    CMD_LS,
    CMD_CD,
    CMD_RM,
    CMD_MKDIR,
    CMD_RMDIR,
    CMD_PUT,
    CMD_GET,
    CMD_CAT,
    CMD_CREATE,
    CMD_EDIT,
    CMD_STATUS,
    CMD_HELP,
    CMD_EXIT,
} CommandCode;

typedef struct {
    bool is_absolute;//是否是絕對路徑
    char **path_segments;//切碎的路徑陣列
    int segment_count;//總共幾段路徑
} PathResult;

//fs的主要功能
int Interaction(FileSystem *file_system);
//負責切分路徑
PathResult parse_path(const char *arg);


//各項指令




#endif