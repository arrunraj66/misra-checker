static int g(void);
static unsigned long f(int i, int x)
{
    return sizeof(i++) + sizeof(x = 1) + sizeof(g());
}
