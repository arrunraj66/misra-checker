static int fact(int n)
{
    return (n <= 1) ? 1 : (n * fact(n - 1));
}
static int even(int n);
static int odd(int n)
{
    return (n == 0) ? 0 : even(n - 1);
}
static int even(int n)
{
    return (n == 0) ? 1 : odd(n - 1);
}
