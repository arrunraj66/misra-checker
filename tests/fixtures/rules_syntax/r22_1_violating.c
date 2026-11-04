#include <stdio.h>
#include <stdlib.h>
static int leak(void)
{
    char *p = malloc(8U);
    if (p == NULL)
    {
        return 1;
    }
    p[0] = 'x';
    return p[0];
}
static int leak_file(void)
{
    FILE *f = fopen("a.txt", "r");
    return f != NULL;
}
