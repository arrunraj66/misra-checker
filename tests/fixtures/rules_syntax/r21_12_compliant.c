#include <string.h>
static void f(char *d, const char *s)
{
    (void)memcpy(d, s, 4U);
}
