#include <stdio.h>
int f(FILE *fp)
{
    FILE copy = *fp;
    (void)copy;
    return 0;
}
