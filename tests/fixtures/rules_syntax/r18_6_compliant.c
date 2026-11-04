static int global_value;
static int *g;
static int *return_static(void)
{
    static int s = 1;
    return &s;
}
static int *return_global(void)
{
    return &global_value;
}
static void store(int **out, int *p)
{
    int local = 0;
    g = &global_value;
    *out = p;
    (void)local;
}
static int *pass_through(int *p)
{
    return p;
}
