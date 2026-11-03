#include <stdint.h>
static void f(uint16_t a, uint16_t b, uint32_t w)
{
    uint32_t r1 = (a + b) * w;
    int r2 = ((a + b) < w);
    (void)r1; (void)r2;
}
