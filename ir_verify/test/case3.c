#include<stdint.h>

uint32_t a[10];

// 4 次循环后 IndexError: Can not prove index Concat(0, ... ... is within array bounds 10
uint32_t test(uint32_t target)
{
    uint32_t l = 0, r = 9;
    uint32_t retval = -1;

    while(l <= r )
    {
        uint32_t mid = l + (r - l) / 2;
        if(a[mid] == target)
        {
            return mid;
        }
        else if(a[mid] > target)
        {
            r = mid - 1;
        }
        else
        {
            l = mid + 1;
        }
    }
    return -1 ;
}