#include <stdint.h>
enum { ANON_A, ANON_B = 5 };
static void f(uint32_t u, int32_t s, char c, double d)
{
    uint32_t a = u + 1U;
    uint32_t b = u + 1;
    int32_t e = s + 10;
    uint32_t g = (c == 'a') ? 1U : 0U;
    double h = d * 2.0;
    int32_t q = s + ANON_B;
    (void)q;
    (void)a; (void)b; (void)e; (void)g; (void)h; (void)(s < 10);
}
