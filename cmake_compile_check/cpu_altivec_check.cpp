#ifndef __APPLE__
#include <altivec.h>
#endif

int foo(void)
{
	unsigned char *x;

	vec_ld(0, x);
}

int main(int a, char **b)
{
	return 0;
}
