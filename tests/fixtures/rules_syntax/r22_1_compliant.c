#include <stdio.h>
#include <stdlib.h>
static char *g;
static int freed(void)
{
    char *p = malloc(8U);
    if (p == NULL)
    {
        return 1;
    }
    free(p);
    return 0;
}
static char *returned(void)
{
    char *p = malloc(8U);
    return p;
}
static void stored(void)
{
    char *p = malloc(8U);
    g = p;
}
static int closed(void)
{
    FILE *f = fopen("a.txt", "r");
    if (f == NULL)
    {
        return 1;
    }
    (void)fclose(f);
    return 0;
}
