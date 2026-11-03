#include <stddef.h>
typedef void (*fn_t)(void);
typedef int (*other_t)(int);
static void target(void) { }
static void f(void)
{
    fn_t p = target;
    other_t q = (other_t)p;
    fn_t n = NULL;
    (void)q; (void)n;
}
