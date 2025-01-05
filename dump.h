#ifndef DUMP_H
#define DUMP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "space.h"
#include "inode.h"

#define PASSWORD_SIZE 64 // 定義密碼大小

void xor_encrypt_decrypt(unsigned char *data, size_t size, const char *password);
void dump_memory(const unsigned char *memory_space, size_t size, const char *password);
unsigned char *load_memory(const char *password);
void create_dump(FileSystem *fs);
unsigned char *read_dump();
#endif