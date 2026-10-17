#include <stdio.h>
void f(void)
{
    FILE *a = fopen("data.txt", "r");
    FILE *b = fopen("data.txt", "w");
    (void)fclose(a);
    (void)fclose(b);
}
