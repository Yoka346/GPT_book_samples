#include <stdio.h>
#include <inttypes.h>

#include "gpt.h"

int main(void)
{
    DiskHandle dh = open_disk(0);
    GPTHeader header;
    if(!load_gpt_header(dh, &header))
        return 0;

    printf("Signature: %s\n", header.signature);
    printf("Revision: %"PRIu16".%"PRIu16"\n", header.revision_upper, header.revision_lower);
    printf("HeaderSize: %"PRIu32" bytes\n", header.header_size);
    printf("HeaderCRC32: %"PRIu32"\n", header.header_crc32);
    printf("MyLBA: %"PRIu64"\n", header.my_lba);
    printf("AlternateLBA: %"PRIu64"\n", header.alternate_lba);
    printf("FirstUsableLBA: %"PRIu64"\n", header.first_usable_lba);
    printf("LastUsableLBA: %"PRIu64"\n", header.last_usable_lba);

    char guid[UUID_STR_LEN];
    uuid_unparse_upper(header.disk_guid, guid);

    printf("DiskGUID: %s\n", guid);
    printf("PartitionEntryLBA: %"PRIu64"\n", header.partition_entry_lba);
    printf("NumberOfPartitionEntry: %"PRIu32"\n", header.num_of_partition_entry);
    printf("SizeOfPartitionEntry: %"PRIu32" bytes\n", header.size_of_partition_entry);
    printf("PartitionEntryArrayCRC32: %"PRIu32"\n", header.partition_entry_array_crc32);

    return 0;
}