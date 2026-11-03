static void f(int *ip, const int *cip)
{
    const int *a = (const int *)ip;
    int *b = (int *)ip;
    const int *c = (const int *)cip;
    (void)a; (void)b; (void)c;
}
