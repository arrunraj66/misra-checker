static int f(int x)
{
    int r = 0;
    if (1)
    {
        r = 1;
    }
    while (0)
    {
        r = 2;
    }
    r = (1 ? 2 : 3);
    do
    {
        r++;
    } while (0);
    if (sizeof(int) == 4U)
    {
        r = x;
    }
    return r;
}
