#include "utils_dump.h"

void dump(uint8_t *data, uint32_t data_len)
{
    uint32_t count = 0;
    printf("-----------------------");
    while(count<data_len)
    {
        if (count %10 == 0){
            printf("\r\n%5d:  ", (count/10));
        }
        printf("%02X\t", data[count++]);
    }
    printf("\r\n-----------------------\r\n");
}
