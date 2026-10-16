/* An octal constant in a macro body is reported even when the macro is never expanded. */
#define MASK 0777
int f(void)
{
    return 1;
}
