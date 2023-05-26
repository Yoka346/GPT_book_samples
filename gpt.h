#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <uchar.h>

#include <uuid/uuid.h>

#include "disk.h"

#define GPT_SIGNATURE "EFI PART"

/**
 * @struct
 * @brief GPTヘッダ構造体
 **/
typedef struct
{
    char signature[8];
    uint16_t revision_lower;
    uint16_t revision_upper;
    uint32_t header_size;
    uint32_t header_crc32;
    uint64_t my_lba;
    uint64_t alternate_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uuid_t disk_guid;
    uint64_t partition_entry_lba;
    uint32_t num_of_partition_entry;
    uint32_t size_of_partition_entry;
    uint32_t partition_entry_array_crc32;
} GPTHeader;

/**
 * @fn
 * @brief 指定されたディスクからGPTヘッダを読み込む. 
 * @param dh 読み込み元のディスクハンドル．
 * @param header 書き込み先のGPTHeader構造体のポインタ．
 * @return 成功したらtrue. 失敗したらfalse.
 **/
bool load_gpt_header(DiskHandle dh, GPTHeader* header);

/**
 * @fn
 * @brief GPTHeader構造体の内容をディスク上の表現(バイナリデータ)として書き出す. 
 * @param header GPTHeader構造体.
 * @param buffer 書き出し先.
 * @param buffer_size 書き出し先のサイズ．
 * @return 書き出し先のサイズが足りていればtrue．そうでなければfalse．
**/
bool gpt_header_to_bytes(GPTHeader* header, char* buffer, size_t buffer_size);

/**
 * @struct
 * @brief パーティションエントリ構造体
 **/
typedef struct
{
    uuid_t partition_type_guid;
    uuid_t unique_partition_guid;
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    char16_t partition_name[36];
} PartitionEntry;

/**
 * @fn
 * @brief パーティションテーブルを読み込む.
 * @param dh ディスハンドル．
 * @param header GPTヘッダ．
 * @param table パーティションテーブルの書き込み先.
 * @param num_entry tableの長さ.
 * @return 読み込んだパーティションエントリの数.
 **/
int32_t load_partition_table(DiskHandle dh, GPTHeader* header, PartitionEntry* table, size_t dest_len);

/**
 * @fn
 * @brief 指定されたユニークパーティションGUIDを持つパーティションを削除する.
 * @param dh ディスクハンドル．
 * @param unique_partition_guid 削除対象のパーティションのユニークGUID．
 * @return 削除が成功したらtrue. 失敗したらfalse．
 **/
bool delete_partition(DiskHandle dh, uuid_t unique_partition_guid);