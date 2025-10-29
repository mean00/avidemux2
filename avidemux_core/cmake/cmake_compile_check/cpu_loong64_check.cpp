void foo(void)
{
    asm volatile("addi.w $r30,$r31,1");
}

int main(int a, char **b)
{
    foo();
    return 0;
}
