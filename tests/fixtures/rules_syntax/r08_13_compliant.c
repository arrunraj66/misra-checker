#include <string.h>
static void fill(int *v)
{
    *v = 1;
}
static int forward(int *v)
{
    fill(v);
    return 0;
}
static void copy(char *dst, const char *src, unsigned int n)
{
    (void)memcpy(dst, src, n);
}
static int read_it(int *v)
{
    return *v;
}
static int (*table)(int *) = read_it;
static int sum(const int *v, int n)
{
    return v[0] + n;
}
