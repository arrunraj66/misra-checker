static int f(int x)
{
    int a = x / 2;
    int b = x % 3;
    int c = 2147483646 + 1;
    int d = 1000 * 1000;
    return a + b + c + d;
}
