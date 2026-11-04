static count = 0;
int use(void)
{
    return undeclared_helper(1);
}
static int bare(int x)
{
    if (x > 0)
    {
        return;
    }
    return 0;
}
