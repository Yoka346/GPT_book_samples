#include <stdio.h>

#include "gpt.h"

int main(void)
{
    char target_guid_str[UUID_STR_LEN];
    if(scanf("%36s", target_guid_str) < 0)
        return 1;

    uuid_t target_guid;
    if(uuid_parse(target_guid_str, target_guid) == -1)
    {
        puts("Invalid GUID.");
        return 0;
    }

    DiskHandle dh = open_disk(0);
    if(delete_partition(dh, target_guid))
        puts("success.");
    else
        puts("failed.");

    return 0;
}