#include <stdint.h>
static void f(uint32_t a, uint8_t b)
{
    uint32_t r1 = a << 32U;
    uint32_t r2 = b << 8U;
    uint32_t r3 = a >> -1;
    int32_t r4 = 1 << 40;
    a <<= 33U;
    (void)r1; (void)r2; (void)r3; (void)r4;
}
