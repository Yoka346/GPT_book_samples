#pragma once
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

#define SECTOR_SIZE 512 // 論理セクタサイズ．この値はディスクによるが，多くが512．
                        // 正確な値を調べたければ，ターミナルで "cat /sys/block/sda/queue/logical_block_size" というコマンドを実行し，
                        // その値をSECTOR_SIZEとする．

typedef int DiskHandle; // 読みやすさのために，ディスクのファイルハンドル(int型)にDiskHandleという名前をつけている．

/**
* @brief 引数で指定された番号のデバイスを開き，そのディスクハンドルを返す.
* @return ディスクハンドル．オープンに失敗した場合は-1が返る． 
**/
DiskHandle open_disk(int disk_num);

/**
 * @brief 特定のセクタに移動する．
 * 
 * @param dh ディスクハンドル．
 * @param lba 移動先のセクタの位置(LBA).
 * @param whence セクタの開始位置．位置の指定は次の定数を用いる.
 *               SEEK_SET: 先頭セクタからlba分だけ移動する.
 *               SEEK_CUR: 現在のセクタからlba分だけ移動する．
 * @return 成功した場合，現在のディスク位置をディスクの先頭からのバイト数で返す。 エラーの場合，-1 が返る.
 **/
ssize_t seek_sector(DiskHandle dh, size_t lba, int whence);

/**
 * @brief 現在のセクタの内容を読み込む．読み込んだセクタ数だけ，ディスクのオフセットは進められる．
 * 
 * @param dh ディスクハンドル．
 * @param buffer 書き込み先．長さは SECTOR_SIZE * num_sector でなければならない．
 * @param num_sector 読み込むセクタ数．
 * @return 成功した場合，読み込んだバイト数を返す．読み込みに失敗した場合は-1が返る．
 **/
ssize_t read_sector(DiskHandle dh, char* buffer, size_t num_sector);

/**
 * @brief 現在のセクタにデータを書き込む．データが書き込まれたセクタ数だけ，ディスクのオフセットは進められる．
 * 
 * @param dh ディスクハンドル．
 * @param buffer 書き込むデータ．長さは SECTOR_SIZE * num_sector でなければならない．
 * @param num_sector 読み込むセクタ数．
 * @return 成功した場合，書き込まれたバイト数を返す．書き込みに失敗した場合は-1が返る．
 **/
ssize_t write_to_sector(DiskHandle dh, char* buffer, size_t num_sector);