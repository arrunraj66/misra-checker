#include <string.h>
struct S { int a; };
union U { int i; float f; };
static void f(char *p)
{
    char a[8] = {0};
    struct S s = {0};
    union U u;
    (void)memcpy(a, a, 4U);
    (void)memcpy(p, p, 1U);
    s = s;
    u.i = 1;
    u.i = u.f;
}
