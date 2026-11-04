static int f(int i)
{
    int a[4] = {0, 1, 2, 3};
    int *p = &a[4];
    int *q = a + 4;
    int *r = a + 0;
    (void)p; (void)q; (void)r;
    return a[3] + a[i];
}
