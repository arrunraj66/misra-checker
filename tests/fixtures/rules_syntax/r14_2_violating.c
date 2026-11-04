static int step(void);
static void f(int n)
{
    int i;
    int j;
    for (i = 0, j = 0; i < n; i++, j++)
    {
    }
    for (i = 0; i < n; i++)
    {
        i += 2;
    }
    for (i = 0; i++ < n; i++)
    {
    }
    for (i = 0; i < n; i += step())
    {
    }
}
