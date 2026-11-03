static void f(int *ip, const int *cip)
{
    const int *a = (const int *)ip;
    const int *b = cip;
    volatile int *c = (volatile int *)ip;
    (void)a; (void)b; (void)c;
}
