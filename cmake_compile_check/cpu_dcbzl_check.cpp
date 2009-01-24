int main(void)
{
	register long zero = 0;
	char data[1024];

	asm volatile("dcbzl %0, %1" : : "b" (data), "r" (zero));

	return 0;
}