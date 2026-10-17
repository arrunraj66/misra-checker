/* NULL passed as the argument of another macro is still the NULL macro. */
#include <stddef.h>
extern void use(void *p);
#define CALL(p, n) use(p)
void f(void)
{
    CALL(NULL, 1);
}
