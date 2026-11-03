#include <stdint.h>
static void f(uint16_t a, uint16_t b, uint16_t c)
{
    uint16_t r1 = (a + b) + c;
    uint16_t r2 = (a + b) * 2U;
    (void)r1; (void)r2;
}
