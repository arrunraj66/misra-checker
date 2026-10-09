static int counter;
int f(void)
{
    counter++;
    return counter;
}
int g(void)
{
    return counter;
}
