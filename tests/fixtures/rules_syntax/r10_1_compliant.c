#include <stdbool.h>
#include <stdint.h>
static void f(uint32_t u, int32_t s, bool b, char c, double d)
{
    uint32_t r1 = u << 2U;
    uint32_t r2 = r1 & 0xFFU;
    uint32_t r3 = ~r2;
    int32_t r4 = s + 1;
    int32_t r5 = -s;
    bool r6 = b && (u > 3U);
    double r7 = d * 2.0;
    char r8 = c + 1;
    int32_t r9 = (int32_t)u % 3;
    uint32_t r10 = (u > 0U) ? 1U : 0U;
    (void)r3; (void)r4; (void)r5; (void)r6; (void)r7; (void)r8; (void)r9;
    (void)r10;
}
