#include <string.h>
struct S { int a; };
union U { int i; float f; };
static void f(struct S *s, struct S *t, int x)
{
    char a[8] = {0};
    char b[8] = {0};
    union U u;
    (void)memcpy(a, b, 4U);
    (void)memmove(a, a + 1, 3U);
    *s = *t;
    u.i = x;
}
