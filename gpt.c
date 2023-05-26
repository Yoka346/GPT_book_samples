#include "gpt.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include "crc32.h"

/**
 *  @fn 16バイトのchar型配列に書き込まれているGUIDをuuid_t型のデータに変換する関数. 
**/
void load_guid(char* buffer, uuid_t guid)
{
    // 一旦，GUIDを XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX 形式の文字列で表現．
    char guid_str[UUID_STR_LEN];
    int str_loc = 0;
    str_loc += sprintf(guid_str, "%08x-", *(uint32_t*)buffer);

    buffer += 4;
    str_loc += sprintf(guid_str + str_loc, "%04x-", *(uint16_t*)buffer & 0xffff);

    buffer += 2;
    str_loc += sprintf(guid_str + str_loc, "%04x-", *(uint16_t*)buffer & 0xffff);
    
    buffer += 2;
    str_loc += sprintf(guid_str + str_loc, "%02x%02x-", buffer[0] & 0xff, buffer[1] & 0xff);

    buffer += 2;
    for(int i = 0; i < 6; i++)
        str_loc += sprintf(guid_str + str_loc, "%02x", buffer[i] & 0xff);

    // 文字列化したGUIDをuuid_parseで解釈して，uuid_t型のデータにする.
    uuid_parse(guid_str, guid);
}

/**
 *  @fn uuid_t型のデータを16バイトのバイナリデータとして書き出す.
**/
void guid_to_bytes(uuid_t guid, char* buffer)
{
    char guid_str[UUID_STR_LEN];
    uuid_unparse_lower(guid, guid_str);

    unsigned int buffer_int[16];
    // %xと指定した場合，自動的にunsigned intにキャストされるので，まずはunsigned int型の配列に値を格納していく.
    sscanf(guid_str, 
          "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
          &buffer_int[3], &buffer_int[2], &buffer_int[1], &buffer_int[0],
          &buffer_int[5], &buffer_int[4],
          &buffer_int[7], &buffer_int[6],
          &buffer_int[8], &buffer_int[9],
          &buffer_int[10], &buffer_int[11], &buffer_int[12], &buffer_int[13], &buffer_int[14], &buffer_int[15]);

    for(int i = 0; i < 16; i++)
        buffer[i] = buffer_int[i];
}

bool load_gpt_header(DiskHandle dh, GPTHeader* header)
{
    seek_sector(dh, 1, SEEK_SET);   // LBA = 1にシーク.

    char sector[SECTOR_SIZE];
    read_sector(dh, sector, 1);     // GPTヘッダがあるセクタを読み込む.

    // シグネチャを確認して，GPTでフォーマットされているかどうか確認.
    if(strncmp(sector, GPT_SIGNATURE, 8) != 0)
    {
        fputs("Error: No GPT Disk.\n", stderr);
        return false;
    }

    // 以下， GPTヘッダ内の情報の読み出し.
    char* buffer = sector;

    memcpy(header->signature, buffer, 8);

    buffer += 8;
    memcpy(&header->revision_lower, buffer, 2);

    buffer += 2;
    memcpy(&header->revision_upper, buffer, 2);

    buffer += 2;
    memcpy(&header->header_size, buffer, 4);

    buffer += 4;
    memcpy(&header->header_crc32, buffer, 4);

    buffer += 4;    // reservedの読み飛ばし.

    buffer += 4;
    memcpy(&header->my_lba, buffer, 8);

    buffer += 8;
    memcpy(&header->alternate_lba, buffer, 8);

    buffer += 8;
    memcpy(&header->first_usable_lba, buffer, 8);

    buffer += 8;
    memcpy(&header->last_usable_lba, buffer, 8);

    buffer += 8;
    uuid_t guid;
    load_guid(buffer, guid);
    uuid_copy(header->disk_guid, guid);

    buffer += 16;
    memcpy(&header->partition_entry_lba, buffer, 8);

    buffer += 8;
    memcpy(&header->num_of_partition_entry, buffer, 4);

    buffer += 4;
    memcpy(&header->size_of_partition_entry, buffer, 4);

    buffer += 4;
    memcpy(&header->partition_entry_array_crc32, buffer, 4);

    return true;
}

bool gpt_header_to_bytes(GPTHeader* header, char* buffer, size_t buffer_size)
{
    if(buffer_size < header->header_size)
        return false;

    memcpy(buffer, header->signature, 8);

    buffer += 8;
    memcpy(buffer, &header->revision_lower, 2);

    buffer += 2;
    memcpy(buffer, &header->revision_upper, 2);

    buffer += 2;
    memcpy(buffer, &header->header_size, 4);

    buffer += 4;
    memcpy(buffer, &header->header_crc32, 4);

    buffer += 4;    
    for(int i = 0; i < 4; i++)  // reserved
        buffer[i] = 0;

    buffer += 4;
    memcpy(buffer, &header->my_lba, 8);

    buffer += 8;
    memcpy(buffer, &header->alternate_lba, 8);

    buffer += 8;
    memcpy(buffer, &header->first_usable_lba, 8);

    buffer += 8;
    memcpy(buffer, &header->last_usable_lba, 8);

    buffer += 8;
    char guid[16];
    guid_to_bytes(header->disk_guid, guid);
    memcpy(buffer, guid, 16);

    buffer += 16;
    memcpy(buffer, &header->partition_entry_lba, 8);

    buffer += 8;
    memcpy(buffer, &header->num_of_partition_entry, 4);

    buffer += 4;
    memcpy(buffer, &header->size_of_partition_entry, 4);

    buffer += 4;
    memcpy(buffer, &header->partition_entry_array_crc32, 4);

    return true;
}

void load_partition_entry(char* buffer, PartitionEntry* entry)
{
    uuid_t guid;

    load_guid(buffer, guid);
    uuid_copy(entry->partition_type_guid, guid);

    buffer += 16;
    load_guid(buffer, guid);
    uuid_copy(entry->unique_partition_guid, guid);

    buffer += 16;
    memcpy(&entry->starting_lba, buffer, 8);

    buffer += 8;
    memcpy(&entry->ending_lba, buffer, 8);

    buffer += 8;
    memcpy(&entry->attributes, buffer, 8);

    buffer += 8;
    memcpy(&entry->partition_name, buffer, 72);
}

int32_t load_partition_table(DiskHandle dh, GPTHeader* header, PartitionEntry* table, size_t dest_len)
{
    if(strncmp(header->signature, GPT_SIGNATURE, 8) != 0)
    {
        fputs("Error: Invalid GPTHeader object.\n", stderr);
        return 0;
    }

    int32_t entry_size = header->size_of_partition_entry;
    int32_t num_entry = header->num_of_partition_entry;
    size_t table_size = entry_size * num_entry;
    uint64_t num_sector = table_size / SECTOR_SIZE;
    if(table_size % SECTOR_SIZE != 0)
        num_sector++;

    char* buffer = (char*)malloc(SECTOR_SIZE * num_sector);
    char* buffer_head = buffer; // 後でバッファを開放するために，先頭ポインタを保持しておく.
    seek_sector(dh, header->partition_entry_lba, SEEK_SET);
    read_sector(dh, buffer, num_sector);

    int32_t read_count;
    for(read_count = 0; read_count < num_entry && read_count < dest_len; read_count++)
    {
        load_partition_entry(buffer, table + read_count);
        buffer += entry_size;
    }

    free(buffer_head);

    return read_count;
}

/**
 * @fn 
 * @brief GPTヘッダのCRC32チェックサムを求めて，その値をGPTヘッダに書き込む.
**/
void modify_gpt_header_checksum(DiskHandle dh)
{
    // GPTヘッダの読み込み．
    GPTHeader header;
    if(!load_gpt_header(dh, &header))
        return;

    // 既にGPTヘッダに書き込まれているチェックサムを0埋め.
    header.header_crc32 = 0;

    // CRC32チェックサムの計算.
    char header_bytes[SECTOR_SIZE];
    memset(header_bytes, 0, SECTOR_SIZE);
    gpt_header_to_bytes(&header, header_bytes, SECTOR_SIZE);
    header.header_crc32 = compute_crc32(header_bytes, header.header_size);

    // ディスクに書き込み.
    gpt_header_to_bytes(&header, header_bytes, SECTOR_SIZE);
    seek_sector(dh, 1, SEEK_SET);
    write_to_sector(dh, header_bytes, 1);
}

/**
 * @fn 
 * @brief パーティションテーブルのCRC32チェックサムを求めて，その値をGPTヘッダに書き込む.
**/
void modify_partition_table_checksum(DiskHandle dh)
{
    // GPTヘッダの読み込み．
    GPTHeader header;
    if(!load_gpt_header(dh, &header))
        return;

    // パーティションテーブルの読み込み
    size_t table_size = header.size_of_partition_entry * header.num_of_partition_entry;
    int32_t sector_num = table_size / SECTOR_SIZE;
    if(table_size % SECTOR_SIZE != 0)
        sector_num++;

    char* table = (char*)malloc(SECTOR_SIZE * sector_num);
    seek_sector(dh, header.partition_entry_lba, SEEK_SET);
    read_sector(dh, table, sector_num);

    // CRC32チェックサムを計算.
    header.partition_entry_array_crc32 = compute_crc32(table, table_size);
    free(table);

    // ディスクに書き込み．
    char header_bytes[SECTOR_SIZE];
    memset(header_bytes, 0, SECTOR_SIZE);
    gpt_header_to_bytes(&header, header_bytes, header.header_size);
    seek_sector(dh, 1, SEEK_SET);
    write_to_sector(dh, header_bytes, 1);
}

/**
 * @fn
 * @brief GPTヘッダとパーティションテーブルをバックアップする.
**/
void backup_gpt(DiskHandle dh)
{
    // GPTヘッダの読み込み．
    GPTHeader header;
    if(!load_gpt_header(dh, &header))
        return;

    // パーティションテーブルの読み込み
    size_t table_size = header.size_of_partition_entry * header.num_of_partition_entry;
    int32_t table_sector_num = table_size / SECTOR_SIZE;
    if(table_size % SECTOR_SIZE != 0)
        table_sector_num++;

    char* table = (char*)malloc(SECTOR_SIZE * table_sector_num);
    seek_sector(dh, header.partition_entry_lba, SEEK_SET);
    read_sector(dh, table, table_sector_num);

    // GPTヘッダのバックアップを読み込み.
    char alternate_header[SECTOR_SIZE];
    seek_sector(dh, header.alternate_lba, SEEK_SET);    
    read_sector(dh, alternate_header, 1);

    // パーティションテーブルのバックアップの先頭セクタをシーク.
    uint64_t alternate_entry_lba = 0;
    memcpy(&alternate_entry_lba, alternate_header + 72, 8);
    seek_sector(dh, alternate_entry_lba, SEEK_SET);

    // パーティションテーブルをバックアップに書き込む.
    write_to_sector(dh, table, table_sector_num);
    free(table);

    // 以下, GPTヘッダのバックアップを作成.

    // headerのMyLBAの値とAlternateLBAの値を交換.
    uint64_t tmp = header.my_lba;
    header.my_lba = header.alternate_lba;
    header.alternate_lba = tmp;

    // headerのPartitionEntryLBAの値をalternate_entry_lbaに変更.
    header.partition_entry_lba = alternate_entry_lba;

    // headerのCRC32チェックサムを計算.
    header.header_crc32 = 0;
    char header_bytes[SECTOR_SIZE];
    memset(header_bytes, 0, SECTOR_SIZE);
    gpt_header_to_bytes(&header, header_bytes, SECTOR_SIZE);
    header.header_crc32 = compute_crc32(header_bytes, header.header_size);

    // GPTヘッダのバックアップに書き込む.
    gpt_header_to_bytes(&header, header_bytes, SECTOR_SIZE);
    seek_sector(dh, header.my_lba, SEEK_SET);    
    write_to_sector(dh, header_bytes, 1);
}

bool delete_partition(DiskHandle dh, uuid_t unique_partition_guid)
{
    // GPTヘッダの読み込み.
    GPTHeader header;
    if(!load_gpt_header(dh, &header))
        return false;

    // パーティションテーブルの読み込み.
    size_t table_size = sizeof(PartitionEntry) * header.num_of_partition_entry;
    PartitionEntry* table = (PartitionEntry*)malloc(table_size);
    load_partition_table(dh, &header, table, table_size);

    // 削除対象のエントリを探す.
    size_t idx;
    for(idx = 0; idx < header.num_of_partition_entry; idx++)
        if(uuid_compare(table[idx].unique_partition_guid, unique_partition_guid) == 0)
            break;

    if(idx == header.num_of_partition_entry)   
        return false;   // 削除対象のエントリが見つからなかった.

    // 削除対象のエントリが書き込まれているセクタを読み込む.
    size_t offset = idx * header.size_of_partition_entry;
    uint64_t target_lba = header.partition_entry_lba + offset / SECTOR_SIZE;
    offset %= SECTOR_SIZE;

    char buffer[SECTOR_SIZE];
    seek_sector(dh, target_lba, SEEK_SET);
    read_sector(dh, buffer, 1);

    // 削除対象のエントリを0埋めする.
    for(size_t i = 0; i < header.size_of_partition_entry; i++)
        buffer[offset + i] = 0;

    // bufferをディスクに書き出す.
    seek_sector(dh, target_lba, SEEK_SET);
    write_to_sector(dh, buffer, 1);

    // パーティションテーブルのチェックサムを再計算.
    modify_partition_table_checksum(dh);
    
    // GPTヘッダのチェックサムを再計算.
    modify_gpt_header_checksum(dh);

    // GPTのバックアップ
    backup_gpt(dh);

    return true;
}