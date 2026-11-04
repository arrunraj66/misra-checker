#include <stdio.h>
static void f(void)
{
    FILE *w = fopen("a.txt", "w");
    FILE *u = fopen("b.txt", "r+");
    (void)fputs("hi", w);
    (void)fputs("hi", u);
}
