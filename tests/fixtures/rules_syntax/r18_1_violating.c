static int f(void)
{
    int a[4] = {0, 1, 2, 3};
    int *p = &a[5];
    int *q = a + 5;
    int *r = a - 1;
    (void)p; (void)q; (void)r;
    return a[4] + a[-1];
}
