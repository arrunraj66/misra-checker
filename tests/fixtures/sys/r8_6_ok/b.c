#include <stdarg.h>
extern int shared_value;
int read_shared(void)
{
    return shared_value;
}
static int variadic(int n, ...)
{
    va_list ap;
    va_start(ap, n);
    va_end(ap);
    return n;
}
static int (*keep)(int, ...) = variadic;
