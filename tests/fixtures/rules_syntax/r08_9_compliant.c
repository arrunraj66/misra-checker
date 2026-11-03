static int shared;
static int a(void)
{
    return shared;
}
static int b(void)
{
    return shared + 1;
}
