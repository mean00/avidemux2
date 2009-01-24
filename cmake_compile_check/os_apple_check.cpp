int main(int a, char **b)
{
#if defined(__APPLE__)
	return 0;
#else
#error GCC is not Apple
#endif
}
