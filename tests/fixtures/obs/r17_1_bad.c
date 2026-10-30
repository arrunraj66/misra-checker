#include <stdarg.h>
int f(int n, ...)
{
    va_list ap;
    int v;
    va_start(ap, n);
    v = va_arg(ap, int);
    va_end(ap);
    return v;
}
