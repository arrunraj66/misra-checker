static int f(int n)
{
    int r = 0;
    switch (n)
    {
    case 0:
        r = 1;
    case 1:
        r = 2;
        break;
    default:
        r = 3;
    }
    return r;
}
