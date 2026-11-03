#include <stdbool.h>
#include <stdint.h>
static void f(uint32_t u, int32_t s, bool b, char c)
{
    int32_t r1 = b + 1;
    uint32_t r2 = -u;
    int32_t r3 = s & 3U;
    bool r4 = s && b;
    int32_t r5 = c * 2;
    (void)r1; (void)r2; (void)r3; (void)r4; (void)r5;
}
