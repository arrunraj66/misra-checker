static void walk(void (*cb)(void))
{
    cb();
}
static void again(void)
{
    walk(again);
}
