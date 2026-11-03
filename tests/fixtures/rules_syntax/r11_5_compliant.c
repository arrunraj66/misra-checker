#include <stddef.h>
static void f(int *ip, void *vp)
{
    void *a = vp;
    void *b = ip;
    int *n = NULL;
    (void)a; (void)b; (void)n;
}
