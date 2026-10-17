#include <fenv.h>
void f(void)
{
    (void)feclearexcept(FE_ALL_EXCEPT);
}
