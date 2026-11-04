static int f(int x)
{
    return x;
    x++;
}
static int g(int x)
{
    if (1)
    {
        return 1;
    }
    else
    {
        x = 2;
    }
    return x;
}
static int h(int x)
{
    switch (x)
    {
    case 1:
        return 1;
        break;
    default:
        return 0;
    }
}
