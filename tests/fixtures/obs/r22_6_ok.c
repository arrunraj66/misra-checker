#include <stdio.h>
void f(FILE *a)
{
    (void)fputs("x", a);
    (void)fclose(a);
}
