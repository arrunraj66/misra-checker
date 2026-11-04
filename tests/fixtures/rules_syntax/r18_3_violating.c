static int f(void)
{
    int a[4] = {0, 1, 2, 3};
    int b[4] = {0, 1, 2, 3};
    return (&a[0] < &b[0]) || (a >= b);
}
