int foo(void)
{
	asm volatile ("movdqa %xmm8, 0");
}

int main(int a, char **b)
{
	return 0;
}
