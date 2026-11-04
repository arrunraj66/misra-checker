static int f(int *p, int *q)
{
    int *a = p + 1;
    int *b = p - 1;
    long d = q - p;
    p += 2;
    return *a + *b + (int)d;
}
