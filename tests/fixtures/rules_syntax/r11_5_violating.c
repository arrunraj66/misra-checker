static void f(void *vp)
{
    int *a = vp;
    char *b = (char *)vp;
    (void)a; (void)b;
}
