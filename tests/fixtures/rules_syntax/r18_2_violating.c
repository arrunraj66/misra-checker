static long f(void)
{
    int a[4] = {0, 1, 2, 3};
    int b[4] = {0, 1, 2, 3};
    return &a[1] - &b[0];
}
