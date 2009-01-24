int main(int a, char **b)
{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)
	return 0;
#else
#error Unable to detect GCC for BSD family
#endif
}
