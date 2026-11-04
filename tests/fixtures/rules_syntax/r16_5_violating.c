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
    case 2:
        r = 3;
        break;
    }
    return r;
}
