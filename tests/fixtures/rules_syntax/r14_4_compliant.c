#include <stdbool.h>
static int f(int x, bool flag)
{
    int r = 0;
    if ((x > 0) && !flag)
    {
        r = 1;
    }
    if (flag)
    {
        r = 2;
    }
    while (x != 0)
    {
        x--;
    }
    do
    {
        r++;
    } while (false);
    for (;;)
    {
        break;
    }
    return r;
}
