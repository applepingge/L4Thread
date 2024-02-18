#include<stdint.h>

uint32_t a[10];

void test()
{
    uint32_t i, j, min;
    for(i = 0; i < 5; i++)
    {
        min = i;
        for(j = 0; j < 5; j++)
        {
            if(j > i && a[j] < a[min])
            //if(a[j] < a[min])
                min = j;
        }
        uint32_t tmp = a[i];
        a[i] = a[min];
        a[min] = tmp;
    }
}