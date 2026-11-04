static int f(void)
{
    return 1;
}
static void use(void)
{
    int r = f();
    (void)f();
    if (f() != 0)
    {
        r++;
    }
    (void)r;
}
