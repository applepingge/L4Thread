#include <stdint.h>
uint32_t z;
int test(uint32_t x, uint32_t y)
{
	z = x + y;
	return 0;
}
