#include <stdint.h>
static void take(uint8_t v) { (void)v; }
static uint8_t ret(uint32_t u) { return u; }
static void f(uint32_t u, int32_t s)
{
    uint8_t n = u;
    int32_t m = u;
    uint8_t big = 300;
    float ff = 1.0;
    char ch = s;
    take(u);
    (void)n; (void)m; (void)big; (void)ff; (void)ch; (void)ret(u);
}
