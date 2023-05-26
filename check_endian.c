#include <stdio.h>

int main(void)
{
    int x = 1;
    char* x_bytes = (char*)&x;
    if(x_bytes[0] == 1)
        puts("little endian.");
    else
        puts("big endian.");

    return 0;
}