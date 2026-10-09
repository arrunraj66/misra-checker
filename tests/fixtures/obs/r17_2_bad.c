int f(int n)
{
    int r = 0;
    if (n != 0)
    {
        r = f(n - 1);
    }
    return r;
}
