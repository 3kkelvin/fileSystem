#ifndef DUMP_H
#define DUMP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "space.h"
#include "inode.h"

#define PASSWORD_SIZE 64 // 定義密碼大小

bool create_dump(FileSystem *fs);
bool save_to_dumpfile(FileSystem* fs, const char* filename, const char *password);
void xor_encrypt_decrypt(unsigned char *data, size_t size, const char *password);
FileSystem *read_dump();
unsigned char *load_memory(const char *password);
#endif