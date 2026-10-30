#include <stdlib.h>
void f(void)
{
    void *p = malloc(4);
    free(p);
}
