static void f(int *ip, unsigned long ul)
{
    unsigned long a = (unsigned long)ip;
    int *b = (int *)ul;
    (void)a; (void)b;
}
