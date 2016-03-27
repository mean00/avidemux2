int foo(void)
{

	asm volatile ("mov lr,pc " : : );
}

int main(int a, char **b)
{
	return 0;
}
