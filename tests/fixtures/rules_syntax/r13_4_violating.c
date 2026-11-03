static void sink(int v)
{
    (void)v;
}
static int f(int a, int b)
{
    int c;
    int d;
    c = d = a;
    if ((a = b) != 0)
    {
        a++;
    }
    sink(a = 1);
    a = (b > 0) ? (c = 1) : 0;
    return c;
}
