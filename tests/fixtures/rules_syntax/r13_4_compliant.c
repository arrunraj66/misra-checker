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
    return a + c;
}
