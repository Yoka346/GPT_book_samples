#include "crc32.h"

#include <stdbool.h>

uint32_t TABLE[256];    // CRC32チェックサムを計算する際に用いるテーブル.
bool table_was_initialized = false; // TABLEが初期化されたかどうか.

void init_table()
{
    for(int i = 0; i < sizeof(TABLE) / sizeof(uint32_t); i++)
    {
        uint32_t x = i;
        for(int j = 0; j < 8; j++)
            x = (x & 1) ? -306674912 ^ x >> 1 : x >> 1;
        TABLE[i] = x;
    }
}

uint32_t compute_crc32(char* data, size_t data_size)
{
    if(!table_was_initialized)
        init_table();

    uint32_t num = UINT32_MAX;
    for(size_t i = 0; i < data_size; i++)
        num = TABLE[(num ^ data[i]) & 255] ^ (num >> 8);
    return num ^ -1;
}