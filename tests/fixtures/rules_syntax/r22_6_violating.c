#include <stdio.h>
static int f(void)
{
    FILE *s = fopen("a.txt", "r");
    (void)fclose(s);
    return fgetc(s);
}
