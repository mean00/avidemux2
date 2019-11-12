void foo(void)
{
	asm volatile ("movdqa %xmm7, 0");
}

int main(int a, char **b)
{
	return 0;
}
