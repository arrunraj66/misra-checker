static void take(int a[4])
{
    (void)a;
}
static void use(void)
{
    int small[2];
    int big[8];
    take(big);
    take(small);
}
