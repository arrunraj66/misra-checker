struct s { int a; };
static void f(int *ip)
{
    char *a = (char *)ip;
    long *b = (long *)ip;
    struct s *c = (struct s *)ip;
    (void)a; (void)b; (void)c;
}
