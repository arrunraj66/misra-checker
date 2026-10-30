int f(void)
{
    int a[3] = {0, 0, 0};
    return &a[0] < &a[1];
}
