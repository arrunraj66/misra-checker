#include <stdlib.h>
static void f(void)
{
    char *p = malloc(4U);
    free(p);
}
