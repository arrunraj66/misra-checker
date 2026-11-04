static int fact_iter(int n)
{
    int r = 1;
    int i;
    for (i = 2; i <= n; i++)
    {
        r *= i;
    }
    return r;
}
static int use(void)
{
    return fact_iter(5);
}
