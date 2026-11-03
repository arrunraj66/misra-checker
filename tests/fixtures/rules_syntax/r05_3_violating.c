typedef int myint;
struct node { int a; };
static int counter;
static int f(int counter)
{
    int total = 0;
    {
        int total = 1;
        total++;
    }
    for (int i = 0; i < 3; i++)
    {
        int counter = 2;
        (void)counter;
    }
    return total;
}
static int g(void)
{
    int myint = 1;
    return myint;
}
static void h(void)
{
    struct node { int b; };
    struct node n;
    (void)n;
}
