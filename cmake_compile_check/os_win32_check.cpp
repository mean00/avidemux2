int main(int a, char **b)
{
#if defined(_WIN32)
	return 0;
#else
#error GCC is not Win32
#endif
}
