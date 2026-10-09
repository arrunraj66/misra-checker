long f(void)
{
    int a[3] = {0, 0, 0};
    int b[3] = {0, 0, 0};
    return &a[1] - &b[0];
}
