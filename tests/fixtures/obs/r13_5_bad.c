int g(void);
int f(int a)
{
    return a && g();
}
