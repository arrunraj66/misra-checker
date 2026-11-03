#include <sys/stat.h>
#define PAIR(a, b) ((a), (b))
static int f(void)
{
    return PAIR(1, 2);
}
static int g(void)
{
    return S_IRUSR;
}
