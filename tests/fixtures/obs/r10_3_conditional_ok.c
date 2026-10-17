/* The essential type of ?: comes from its operands, not from the promoted C type. */
unsigned char a, b, c;
signed char s, t;
void f(void)
{
    s = (t > 0) ? t : s;
    c = (a > b) ? a : b;
}
