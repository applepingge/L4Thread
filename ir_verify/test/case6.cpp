#include<stdint.h>
class B
{
public:
	int n;
	int m;
};

class A
{
public:
	B b;
};
A a;
int x;
B* get_b(A* n)
{return &(n->b);}

int test(int y)
{	
	x = 0;
	tag
	for(int i = 0; i < y; i++)
	{
		x *= y;
	}
	return x;
}