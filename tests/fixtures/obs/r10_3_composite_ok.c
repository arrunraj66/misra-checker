/* Essential type of small-integer arithmetic is not promoted. */
unsigned char a, b, c;
void f(void)
{
    c = a + b;
}
