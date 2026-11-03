static int f(int x, int *p)
{
    int r = 0;
    if (x)
    {
        r = 1;
    }
    while (x)
    {
        x--;
    }
    if (p)
    {
        r = 2;
    }
    while (1)
    {
        break;
    }
    if (x > 0 ? 1 : 0)
    {
        r = 3;
    }
    if ((r++, x))
    {
        r = 4;
    }
    return r;
}
