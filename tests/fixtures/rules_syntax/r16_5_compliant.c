static int f(int n)
{
    int r = 0;
    switch (n)
    {
    case 1:
        r = 1;
        break;
    default:
        r = 2;
        break;
    }
    switch (n)
    {
    default:
        r = 3;
        break;
    case 2:
        r = 4;
        break;
    }
    switch (n)
    {
    case 3:
    default:
        r = 5;
        break;
    }
    return r;
}
