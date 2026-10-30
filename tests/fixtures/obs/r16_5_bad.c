int f(int a)
{
    switch (a)
    {
        case 1:
            a = 2;
            break;
        default:
            a = 0;
            break;
        case 2:
            a = 3;
            break;
    }
    return a;
}
