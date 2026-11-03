#include <stdlib.h>
static int cmp(const void *a, const void *b) { (void)a; (void)b; return 0; }
void f(int *v)
{
    qsort(v, 2U, sizeof(int), cmp);
}
