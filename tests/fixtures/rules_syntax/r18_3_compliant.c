static int f(int *p, int *q)
{
    int a[4] = {0, 1, 2, 3};
    return (&a[0] < &a[2]) || (p < q);
}
