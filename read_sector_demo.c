#include <stdio.h>

#include "disk.h"

int main(void)
{
    DiskHandle dh = open_disk(0);
    seek_sector(dh, 1, SEEK_SET);   // LBA = 1 に移動.

    char buffer[SECTOR_SIZE];   // 書き込み先.
    read_sector(dh, buffer, 1);     // セクタを読み込む.

    buffer[8] = '\0';   // 8番目の要素に終端文字を格納して，先頭8バイトのみが文字列として表示されるようにする．
    puts(buffer);

    return 0;
}