#include <stdbool.h>
#include <stdint.h>
static uint8_t g(uint8_t v) { return v; }
static void f(uint8_t a, uint32_t u)
{
    uint8_t n1 = 5U;
    uint8_t n2 = 5;
    uint32_t w = a;
    int32_t i = 7;
    double d = 1.0;
    float fl = 1.0f;
    bool b1 = true;
    bool b2 = (u > 3U);
    char ch = 'x';
    uint8_t viaCall = g(a);
    (void)n1; (void)n2; (void)w; (void)i; (void)d; (void)fl; (void)b1;
    (void)b2; (void)ch; (void)viaCall;
}
