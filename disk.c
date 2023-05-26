#include "disk.h"

#include <stdio.h>
#include <fcntl.h>

DiskHandle open_disk(int disk_num)
{
    char path[128];
    sprintf(path, "/dev/sd%c", 'a' + disk_num); // disk_numから開くディスクのパスを生成.
    return open(path, O_RDWR);  // ファイルと同じ要領でopen関数でファイルディスクリプタを得る.
}

ssize_t seek_sector(DiskHandle dh, size_t lba, int whence)
{
    return lseek(dh, lba * SECTOR_SIZE, whence);    // ファイルをシークする時と全く同じ．
}

ssize_t read_sector(DiskHandle dh, char* buffer, size_t num_sector)
{
    return read(dh, buffer, SECTOR_SIZE * num_sector);    // ファイルをreadする時と全く同じ．
}

ssize_t write_to_sector(DiskHandle dh, char* buffer, size_t num_sector)
{
    return write(dh, buffer, SECTOR_SIZE * num_sector);    // ファイルをwriteする時と全く同じ．
}