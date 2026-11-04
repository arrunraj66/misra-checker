static int f(int *p, int *q)
{
    int r = p[1];
    p++;
    r += *p;
    return r + (p == q);
}
