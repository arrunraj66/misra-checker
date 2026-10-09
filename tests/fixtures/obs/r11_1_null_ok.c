#include <stddef.h>
typedef void (*fp)(void);
int f(fp p)
{
    return p != NULL;
}
