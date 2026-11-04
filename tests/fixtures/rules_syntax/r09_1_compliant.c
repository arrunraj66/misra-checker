static int f(int c)
{
    int x;
    int y = 0;
    int z;
    int w;
    int *p = &w;
    if (c != 0)
    {
        x = 1;
    }
    else
    {
        x = 2;
    }
    z = x + y;
    *p = 1;
    return z + w + (int)sizeof(z);
}
