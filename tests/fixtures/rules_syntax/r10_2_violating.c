#include <stdint.h>
static void f(char c, int32_t s)
{
    int32_t a = c + c;
    int32_t b = s - c;
    (void)a; (void)b;
}
