static unsigned long f(int i, int *a)
{
    return sizeof(i) + sizeof(int) + sizeof(a[0]);
}
