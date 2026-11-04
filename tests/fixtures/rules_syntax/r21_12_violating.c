#include <fenv.h>
static void f(void)
{
    (void)feclearexcept(FE_ALL_EXCEPT);
    (void)fetestexcept(FE_ALL_EXCEPT);
}
