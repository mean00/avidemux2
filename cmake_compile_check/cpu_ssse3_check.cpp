int foo(void)
{
	asm volatile ("pabsw %xmm0, %xmm0");
}

int main(int a, char **b)
{
	return 0;
}
