#include <stdio.h>
void f(FILE *a)
{
    (void)fclose(a);
    (void)fputs("x", a);
}
