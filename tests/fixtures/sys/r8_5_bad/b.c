int fn(void);
static int use(void)
{
    return fn();
}
static int (*keep)(void) = use;
