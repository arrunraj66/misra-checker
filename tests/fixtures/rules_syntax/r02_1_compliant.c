static int f(int x)
{
    int r = 0;
    while (1)
    {
        if (x > 3)
        {
            break;
        }
        x++;
    }
    for (;;)
    {
        if (x > 9)
        {
            break;
        }
        x++;
    }
    switch (x)
    {
    case 1:
        r = 1;
        break;
    default:
        r = 2;
        break;
    }
    return r + x;
}
