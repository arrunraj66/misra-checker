static int g(void);
static void f(int x, int *p)
{
    x = 1;
    x++;
    (void)x;
    g();
    *p = 2;
}
