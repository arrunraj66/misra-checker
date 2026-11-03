static int f(int a, int b)
{
    int c;
    int i;
    c = b;
    a += 2;
    (void)(a = b);
    for (i = 0; i < 3; i = i + 1)
    {
        c++;
    }
    for (i = 0, c = 0; i < 3; i++, c = c + 1)
    {
        a++;
    }
    a = 1, c = 2;
    (b > 0) ? (a = 1) : (c = 2);
    return a + c;
}
