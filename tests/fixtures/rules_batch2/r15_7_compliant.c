int f(int x)
{
    if (x > 0)
    {
        x--;
    }
    if (x < 0)
    {
        x++;
    }
    if (x == 1)
    {
        x = 2;
    }
    else if (x == 2)
    {
        x = 3;
    }
    else
    {
        x = 0;
    }
    return x;
}
