void foo(void)
{
	asm volatile ("mv a0,t0 " : : );
}

int main(int a, char **b)
{
	return 0;
}
