static int h(int a);
static int f(int i)
{
    int x;
    i++;
    x = i;
    (void)h(i);
    return x;
}
