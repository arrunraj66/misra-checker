#include <stdio.h>
static int f(void)
{
    FILE *s = fopen("a.txt", "r");
    int c = fgetc(s);
    (void)fclose(s);
    s = fopen("b.txt", "r");
    c += fgetc(s);
    (void)fclose(s);
    return c;
}
