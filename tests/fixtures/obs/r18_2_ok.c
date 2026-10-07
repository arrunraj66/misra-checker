long f(void)
{
    int a[3] = {0, 0, 0};
    return &a[2] - &a[0];
}
