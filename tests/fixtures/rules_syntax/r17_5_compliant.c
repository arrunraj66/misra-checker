static void take(int a[4])
{
    (void)a;
}
static void use(int *p)
{
    int exact[4];
    int big[8];
    take(exact);
    take(big);
    take(p);
}
