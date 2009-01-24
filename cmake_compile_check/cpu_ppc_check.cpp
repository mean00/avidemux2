int foo(void)
{
	register const void *p;

	asm volatile ("dcbt 0, %0" : : "r" (p));
}

int main(int a, char **b)
{
	return 0;
}
