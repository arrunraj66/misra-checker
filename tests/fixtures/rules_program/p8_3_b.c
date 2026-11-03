extern long shared_var;
int fn(int x, int b);
static long use(void)
{
    return shared_var + fn(1, 2);
}
