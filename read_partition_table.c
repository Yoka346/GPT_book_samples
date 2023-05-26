#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <wchar.h>
#include <locale.h>

#include "gpt.h"

int main(void)
{
    // 文字のロケールを環境に合わせる. uchar16_t文字列の変換を行うため．
    setlocale(LC_CTYPE, "");     

    DiskHandle dh = open_disk(0);
    GPTHeader header;
    if(!load_gpt_header(dh, &header))
        return 0;

    size_t table_len = sizeof(PartitionEntry) * header.num_of_partition_entry;
    PartitionEntry* table = (PartitionEntry*)malloc(table_len);
    load_partition_table(dh, &header, table, table_len);

    uuid_t zero_guid;
    uuid_clear(zero_guid);
    char guid_str[UUID_STR_LEN];
    for(int32_t i = 0; i < header.num_of_partition_entry; i++)
    {
        PartitionEntry* entry = table + i;
        if(uuid_compare(entry->partition_type_guid, zero_guid) == 0)  // 未使用エントリかどうか．
            continue;

        uuid_unparse_upper(entry->partition_type_guid, guid_str);
        printf("PartitionTypeGUID: %s\n", guid_str);

        uuid_unparse_upper(entry->unique_partition_guid, guid_str);
        printf("UniquePartitionGUID: %s\n", guid_str);

        printf("StartingLBA: %"PRIu64"\n", entry->starting_lba);
        printf("EndingLBA: %"PRIu64"\n", entry->ending_lba);
        printf("Attributes: 0x%016"PRIX64"\n", entry->attributes);

        // PartitionEntry::partition_nameはchar16_tの配列であるため，そのまま出力はできない.
        // よって環境に合わせてマルチバイト文字に変換する.
        static const size_t NAME_LEN = sizeof(entry->partition_name) / sizeof(char16_t);
        char name_out[MB_CUR_MAX * NAME_LEN];   // 変換後の文字列.
        int count = 0;
        for(int j = 0; j < NAME_LEN; j++)
        {
            size_t res = c16rtomb(name_out + count, entry->partition_name[j], NULL);
            if(res == (size_t)-1)
                break;
            count += res;
        }
        
        printf("PartitionName: %s\n\n", name_out);
    }

    free(table);

    return 0;
}