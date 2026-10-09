int f(int a)
{
    a = 1;
    return a;
}

int g(int a)
{
    switch (a)
    {
        case 1:
            return 1;
        default:
            break;
    }
    return 0;
}

int h(void)
{
    goto done;
done:
    return 0;
}
