#include <stdint.h>
static void f(char c, int32_t s)
{
    char a = c + 1;
    int32_t b = c - '0';
    char d = c - 1;
    char e = s + c;
    (void)a; (void)b; (void)d; (void)e;
}
