static int pick(int x)
{
    if (x > 0)
    {
        return 1;
    }
    return 0;
}
static void early(int x)
{
    if (x > 0)
    {
        return;
    }
    x++;
}
