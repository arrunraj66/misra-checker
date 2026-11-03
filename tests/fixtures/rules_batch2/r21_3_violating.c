#include <stdlib.h>
void f(void)
{
    void *p = malloc(4U);
    p = realloc(p, 8U);
    free(p);
}
