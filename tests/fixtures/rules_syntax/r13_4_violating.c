static int f(int a, int b)
{
    int c;
    int d;
    c = d = a;
    if ((a = b) != 0)
    {
        a++;
    }
    return c;
}
