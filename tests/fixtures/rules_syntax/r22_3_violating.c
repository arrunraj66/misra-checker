#include <stdio.h>
static void f(void)
{
    FILE *a = fopen("data.txt", "r");
    FILE *b = fopen("data.txt", "w");
    (void)a; (void)b;
}
