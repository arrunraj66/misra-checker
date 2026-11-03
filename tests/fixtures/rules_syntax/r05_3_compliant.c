struct holder { int value; };
static int value;
static void early(void)
{
    int later_name = 0;
    (void)later_name;
}
static int later_name;
static int one(void)
{
    int tmp = 1;
    {
        int inner_a = 2;
        tmp += inner_a;
    }
    {
        int inner_a = 3;
        tmp += inner_a;
    }
    return tmp + value + later_name;
}
typedef int (*callback_t)(int x, int ctx);
static int use_ctx(int ctx)
{
    return ctx;
}
