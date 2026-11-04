static int h(int a, int b);
static int f(int i)
{
    int a[4] = {0, 0, 0, 0};
    int r;
    i = i++ + 1;
    a[i] = i++;
    r = i++ + i;
    r += h(i++, i);
    return r + a[0];
}
