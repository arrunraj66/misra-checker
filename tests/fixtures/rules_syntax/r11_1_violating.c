typedef void (*fn_t)(void);
static void f(void *vp, fn_t p)
{
    void *v1 = (void *)p;
    unsigned long u = (unsigned long)p;
    fn_t q = (fn_t)vp;
    (void)v1; (void)u; (void)q;
}
