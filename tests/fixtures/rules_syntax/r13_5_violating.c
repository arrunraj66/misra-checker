static int g(void);
static int f(int a, int b, int c)
{
    int i = 0;
    int r = (a && (b = c)) || (a || i++);
    return r + (a && g());
}
