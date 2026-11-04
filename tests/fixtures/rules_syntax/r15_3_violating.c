static void f(int x)
{
    if (x > 0)
    {
        goto inner;
    }
    {
inner:
        x++;
    }
}
