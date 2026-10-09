typedef void (*fp)(void);
long f(fp p)
{
    return (long)p;
}
