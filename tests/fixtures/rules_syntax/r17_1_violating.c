#include <stdarg.h>
static int first(int n, ...)
{
    va_list ap;
    int total;
    va_start(ap, n);
    total = va_arg(ap, int);
    va_end(ap);
    return total;
}
