static int h(int a, int b);
static int f(int i, int j)
{
    int a[4] = {0, 0, 0, 0};
    int r;
    i = i + 1;
    a[i] = 1;
    i++;
    r = h(i, j);
    return r + a[0];
}
