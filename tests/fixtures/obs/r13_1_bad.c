int g(void);
void f(void)
{
    int b[2] = { g(), 1 };
    (void)b;
}
