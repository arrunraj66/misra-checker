static int f(int x, int *p)
{
    x = 1;
    x++;
    x += 2;
    p++;
    return x + *p;
}
