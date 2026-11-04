static int g(void);
static int f(int i)
{
    volatile int v = 0;
    int a[2] = {i++, 2};
    int b[1] = {v};
    int c[1] = {g()};
    return a[0] + b[0] + c[0];
}
