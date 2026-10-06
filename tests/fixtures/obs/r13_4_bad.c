int f(int a)
{
    int b;
    if ((b = a) != 0)
    {
        a = 1;
    }
    return a + b;
}
