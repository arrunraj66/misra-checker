struct pair
{
    int a;
    int b;
};
static int first(int *v)
{
    return v[0];
}
static int total(int *v, int n)
{
    int t = 0;
    int i;
    for (i = 0; i < n; i++)
    {
        t += *v++;
    }
    return t;
}
static int field(struct pair *p)
{
    return p->a + p->b;
}
int exported(int *v)
{
    return *v;
}
