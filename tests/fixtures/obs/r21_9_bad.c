#include <stdlib.h>
static int c(const void *a, const void *b)
{
    return a == b;
}
void f(int *v)
{
    qsort(v, 2, sizeof(int), c);
}
