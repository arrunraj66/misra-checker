#include <stdio.h>
static void f(void)
{
    FILE *f = fopen("a.txt", "r");
    (void)fputs("hi", f);
    (void)fprintf(f, "x");
}
