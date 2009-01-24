int main(int a, char **b)
{
#if defined(__MINGW32__)
	return 0;
#else
#error GCC is not MinGW
#endif
}
