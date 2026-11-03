#include <stddef.h>
static void f(int *p)
{
    int *a = NULL;
    if (p == NULL)
    {
        a = p;
    }
    (void)a;
}
