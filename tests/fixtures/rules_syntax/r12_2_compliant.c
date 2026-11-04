#include <stdint.h>
static void f(uint32_t a, uint8_t b, uint32_t n)
{
    uint32_t r1 = a << 3U;
    uint32_t r2 = 1U << 31U;
    uint32_t r3 = a >> 31U;
    uint8_t r4 = (uint8_t)(b << 7U);
    uint32_t r5 = a << n;
    (void)r1; (void)r2; (void)r3; (void)r4; (void)r5;
}
