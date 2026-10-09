static int local_fn(void)
{
    return 2;
}
static int (*keep)(void) = local_fn;
