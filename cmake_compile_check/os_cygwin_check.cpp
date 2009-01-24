int main(int a, char **b)
{
#if defined(__CYGWIN__)
	return 0;
#else
#error GCC is not Cygwin
#endif
}
