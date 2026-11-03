#include <stdint.h>
static void f(uint16_t a, uint16_t b)
{
    uint16_t r1 = (uint16_t)(a + b);
    uint32_t r2 = (uint32_t)a;
    (void)r1; (void)r2;
}
