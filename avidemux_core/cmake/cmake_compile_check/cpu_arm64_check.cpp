void foo(void)
{
    asm volatile("mov x27,x28 " : :);
}

int main(int a, char **b)
{
    foo();
    return 0;
}
