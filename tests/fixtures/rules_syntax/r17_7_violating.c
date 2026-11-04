static int f(void)
{
    return 1;
}
static int g(int x)
{
    return x;
}
static void use(int (*fp)(void))
{
    f();
    g(2);
    fp();
}
