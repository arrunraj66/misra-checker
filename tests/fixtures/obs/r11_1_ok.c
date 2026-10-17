void h(void);
void g(void)
{
    void (*p)(void) = h;
    p();
}
