static void f(void *vp, unsigned long ul)
{
    void *a = (void *)ul;
    unsigned long b = (unsigned long)vp;
    (void)a; (void)b;
}
