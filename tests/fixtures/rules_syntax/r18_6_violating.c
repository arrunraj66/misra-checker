static int *g;
static int *return_local(void)
{
    int x = 1;
    return &x;
}
static int *return_array(void)
{
    int a[3] = {1, 2, 3};
    return a;
}
static void store_global(void)
{
    int y = 2;
    g = &y;
}
static void store_out(int **out)
{
    int z = 3;
    *out = &z;
}
