struct hidden;
static void f(struct hidden *hp)
{
    int *a = (int *)hp;
    unsigned long b = (unsigned long)hp;
    (void)a; (void)b;
}
