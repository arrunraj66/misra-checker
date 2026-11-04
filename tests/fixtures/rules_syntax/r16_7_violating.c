static int f(int n, _Bool b)
{
    int r = 0;
    switch (n > 0)
    {
    case 0:
        r = 1;
        break;
    default:
        break;
    }
    switch (b)
    {
    case 0:
        r = 2;
        break;
    default:
        break;
    }
    return r;
}
