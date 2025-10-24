void foo(void)
{
    asm volatile("mov lr,pc " : :);
}

int main(int a, char **b)
{
    foo();
    return 0;
}
