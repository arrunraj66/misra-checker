#include "h.h"
static int use(void)
{
    return fn();
}
static int (*keep)(void) = use;
