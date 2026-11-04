#include <stdlib.h>
static int my_abs(int x)
{
    int _local = x;
    return _local < 0 ? -_local : _local;
}
