#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "w");
    (void)fputs("y", a);
    (void)fclose(a);
}
