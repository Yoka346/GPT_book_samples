#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>   // PRId32やPRIu32などのマクロを用いるために必要.

int main(void)
{
    // 符号あり
    puts("signed");

    int32_t i32 = 127;
    printf("%"PRId32"\n", i32);  // 出力: 127
    printf("%04"PRId32"\n", i32);  // 出力: 0127
    printf("%"PRIx32"\n", i32);  // 出力: 7f
    printf("%"PRIX32"\n", i32);  // 出力: 7F

    // 符号なし
    puts("\nunsigned");

    uint32_t ui32 = 255;
    printf("%"PRIu32"\n", ui32);  // 出力: 255
    printf("%04"PRIu32"\n", ui32);  // 出力: 0255
    printf("%"PRIx32"\n", ui32);  // 出力: ff
    printf("%"PRIX32"\n", ui32);  // 出力: FF
    return 0;
}