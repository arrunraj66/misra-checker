static int f(int n)
{
    int r = 0;
    switch (n)
    {
    case 0:
    case 1:
        r = 1;
        break;
    default:
        r = 2;
        break;
    }
    return r;
}
