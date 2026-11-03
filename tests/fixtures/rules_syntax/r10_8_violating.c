#include <stdint.h>
static void f(uint16_t a, uint16_t b)
{
    uint32_t r1 = (uint32_t)(a + b);
    int32_t r2 = (int32_t)(a + b);
    float r3 = (float)(a * b);
    (void)r1; (void)r2; (void)r3;
}
