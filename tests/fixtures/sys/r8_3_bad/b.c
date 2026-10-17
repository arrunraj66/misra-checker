static int unused_helper(void)
{
    return 0;
}
static int (*keep)(void) = unused_helper;
