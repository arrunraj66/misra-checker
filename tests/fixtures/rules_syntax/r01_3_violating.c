static int f(int x)
{
    int a = x / 0;
    int b = x % 0;
    int c = 2147483647 + 1;
    int d = 100000 * 100000;
    return a + b + c + d;
}
