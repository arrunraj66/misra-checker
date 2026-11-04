static int h(int a);
static int f(int i)
{
    int a[4] = {0, 0, 0, 0};
    int x;
    int y = i++;
    x = i++;
    a[i++] = 0;
    (void)h(i++);
    return x + y + a[0];
}
