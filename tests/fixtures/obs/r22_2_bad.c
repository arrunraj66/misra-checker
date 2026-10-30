#include <stdlib.h>
void f(void)
{
    int x;
    free(&x);
}
