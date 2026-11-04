#include <stdlib.h>
static int cmp(const void *a, const void *b);
static void sort(int *v)
{
    qsort(v, 2U, sizeof(int), cmp);
}
static int cmp(const void *a, const void *b)
{
    sort(0);
    (void)a;
    (void)b;
    return 0;
}
