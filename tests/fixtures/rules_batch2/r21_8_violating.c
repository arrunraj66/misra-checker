#include <stdlib.h>
void f(void)
{
    (void)getenv("HOME");
    exit(1);
}
