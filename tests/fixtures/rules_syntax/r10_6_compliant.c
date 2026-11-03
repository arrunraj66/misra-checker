#include <stdint.h>
static void f(uint16_t a, uint16_t b)
{
    uint16_t r1 = a + b;
    uint32_t r2 = (uint32_t)a + b;
    uint32_t r3 = a;
    (void)r1; (void)r2; (void)r3;
}
struct bf
{
    unsigned int a : 3;
};
static void g(struct bf *p)
{
    p->a = p->a + 1U;
}
