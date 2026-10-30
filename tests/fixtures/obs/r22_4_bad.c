#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "r");
    (void)fputs("y", a);
    (void)fclose(a);
}
