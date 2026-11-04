#include <stdio.h>
static int g(FILE *f)
{
    FILE copy = *f;
    return copy._flags + f->_flags;
}
