#include <stdint.h>
static void f(uint16_t a, uint16_t b, uint32_t u, float f1, float f2)
{
    uint32_t w = a + b;
    uint64_t x = u * u;
    double dd = f1 * f2;
    (void)w; (void)x; (void)dd;
}
