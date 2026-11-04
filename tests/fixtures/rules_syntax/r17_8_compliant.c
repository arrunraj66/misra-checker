static int f(int x, int *p)
{
    int local = x;
    local++;
    *p = local;
    return local;
}
