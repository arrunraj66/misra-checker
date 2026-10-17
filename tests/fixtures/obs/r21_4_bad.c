#include <setjmp.h>
jmp_buf b;
void f(void)
{
    longjmp(b, 1);
}
