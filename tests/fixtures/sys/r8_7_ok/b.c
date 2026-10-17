int shared_fn(void);
static int user(void)
{
    return shared_fn();
}
static int (*keep)(void) = user;
