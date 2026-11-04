static void run(void (*cb)(void))
{
    cb();
}
static void leaf(void)
{
}
static void use(void)
{
    run(leaf);
}
