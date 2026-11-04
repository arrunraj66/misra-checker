#include <stdlib.h>
static void f(void)
{
    int x = 0;
    char buf[4];
    free(&x);
    free(buf);
    free("literal");
}
