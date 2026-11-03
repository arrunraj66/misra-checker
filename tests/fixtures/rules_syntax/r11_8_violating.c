static void f(const char *ccp, volatile int *vip)
{
    char *a = (char *)ccp;
    int *b = (int *)vip;
    char *c = ccp;
    (void)a; (void)b; (void)c;
}
