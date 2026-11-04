static long f(int *p, int *q)
{
    int a[4] = {0, 1, 2, 3};
    long same = &a[3] - &a[0];
    long unknown = p - q;
    return same + unknown;
}
