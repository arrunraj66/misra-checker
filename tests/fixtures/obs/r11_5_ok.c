#include <stddef.h>
void *g(void);
void *f(void)
{
    int *p = NULL;
    (void)p;
    return g();
}
